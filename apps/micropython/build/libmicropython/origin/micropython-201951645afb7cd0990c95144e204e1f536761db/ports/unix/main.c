/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 * Copyright (c) 2014-2017 Paul Sokolovsky
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

#include "py/compile.h"
#include "py/runtime.h"
#include "py/builtin.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/objstr.h"
#include "py/stackctrl.h"
#include "py/mphal.h"
#include "py/mpthread.h"
#include "extmod/misc.h"
#include "extmod/modplatform.h"
#include "extmod/vfs.h"
#include "extmod/vfs_posix.h"
#include "genhdr/mpversion.h"
#include "input.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <uk/plat/memory.h>
#include <uk/sched.h>

// Command line options, with their defaults
STATIC bool compile_only = false;
STATIC uint emit_opt = MP_EMIT_OPT_NONE;

static int gotime = 0;
unsigned long executions = 0;

#define RUNS 3
#define RUNNING 3
#define RUNTIME 10000000 //10s in microseconds in paper experiment although I've set it to 3 seconds
// here to preserve your sanity: this is a prototype and is veryyyyy unstable (a lot of work is needed)
// to support concurrency well in this OS

#if MICROPY_ENABLE_GC
// Heap size of GC heap (if enabled)
// Make it larger on a 64 bit machine, because pointers are larger.
long heap_size = 1024 * 1024 * (sizeof(mp_uint_t) / 4);
#endif

// Number of heaps to assign by default if MICROPY_GC_SPLIT_HEAP=1
#ifndef MICROPY_GC_SPLIT_HEAP_N_HEAPS
#define MICROPY_GC_SPLIT_HEAP_N_HEAPS (1)
#endif

#if !MICROPY_PY_SYS_PATH
#error "The unix port requires MICROPY_PY_SYS_PATH=1"
#endif

#if !MICROPY_PY_SYS_ARGV
#error "The unix port requires MICROPY_PY_SYS_ARGV=1"
#endif

STATIC void stderr_print_strn(void *env, const char *str, size_t len) {
    (void)env;
    ssize_t ret;
    MP_HAL_RETRY_SYSCALL(ret, write(STDERR_FILENO, str, len), {});
    mp_os_dupterm_tx_strn(str, len);
}

long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

const mp_print_t mp_stderr_print = {NULL, stderr_print_strn};

#define FORCED_EXIT (0x100)
// If exc is SystemExit, return value where FORCED_EXIT bit set,
// and lower 8 bits are SystemExit value. For all other exceptions,
// return 1.
STATIC int handle_uncaught_exception(mp_obj_base_t *exc) {
    // check for SystemExit
    if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
        // None is an exit value of 0; an int is its value; anything else is 1
        mp_obj_t exit_val = mp_obj_exception_get_value(MP_OBJ_FROM_PTR(exc));
        mp_int_t val = 0;
        if (exit_val != mp_const_none && !mp_obj_get_int_maybe(exit_val, &val)) {
            val = 1;
        }
        return FORCED_EXIT | (val & 255);
    }

    // Report all other exceptions
    mp_obj_print_exception(&mp_stderr_print, MP_OBJ_FROM_PTR(exc));
    return 1;
}

#define LEX_SRC_STR (1)
#define LEX_SRC_VSTR (2)
#define LEX_SRC_FILENAME (3)
#define LEX_SRC_STDIN (4)

static int finish = 0;

STATIC int execute_from_lexer_modified(int source_kind, const void *source, mp_parse_input_kind_t input_kind, bool is_repl) {
    mp_hal_set_interrupt_char(CHAR_CTRL_C);

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // create lexer based on source kind
        mp_lexer_t *lex;
        if (source_kind == LEX_SRC_STR) {
            const char *line = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, line, strlen(line), false);
        } else if (source_kind == LEX_SRC_VSTR) {
            const vstr_t *vstr = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, vstr->buf, vstr->len, false);
        } else if (source_kind == LEX_SRC_FILENAME) {
            lex = mp_lexer_new_from_file((const char *)source);
        } else { // LEX_SRC_STDIN
            lex = mp_lexer_new_from_fd(MP_QSTR__lt_stdin_gt_, 0, false);
        }
        
        qstr source_name = lex->source_name;

        // #if MICROPY_PY___FILE__
        // if (input_kind == MP_PARSE_FILE_INPUT) {
        //     mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
        // }
        // #endif

        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);

        

        #if defined(MICROPY_UNIX_COVERAGE)
        // allow to print the parse tree in the coverage build
        if (mp_verbose_flag >= 3) {
            printf("----------------\n");
            mp_parse_node_print(&mp_plat_print, parse_tree.root, 0);
            printf("----------------\n");
        }
        #endif

        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, is_repl);
        unsigned long total_time = 0;
       
        int concurrent = 0;
        unsigned long start_time_total = ustime();
        while (ustime() - start_time_total < RUNTIME) {
            unsigned long start_time = ustime();
            int pid = DO_SFORK();

            if (pid == 0) {

                if (!compile_only) {
                    mp_call_function_0(module_fun);
                }
                sfork_syscalls.sfork_exit(0);

            } 
            concurrent++;

            int statloc;
            int pid1;
            int finished = 0;
            if (concurrent == RUNNING) {
                while(pid1 = sfork_syscalls.sfork_wait3(&statloc,WNOHANG,NULL) == 0) {
                } 
                concurrent--;
                executions++;
            }

        }

        mp_hal_set_interrupt_char(-1);
        mp_handle_pending(true);
        nlr_pop();
        return 0;

    } else {
        // uncaught exception
        mp_hal_set_interrupt_char(-1);
        mp_handle_pending(false);
        return handle_uncaught_exception(nlr.ret_val);
    }
}

// Returns standard error codes: 0 for success, 1 for all other errors,
// except if FORCED_EXIT bit is set then script raised SystemExit and the
// value of the exit is in the lower 8 bits of the return value
STATIC int execute_from_lexer(int source_kind, const void *source, mp_parse_input_kind_t input_kind, bool is_repl) {
    mp_hal_set_interrupt_char(CHAR_CTRL_C);

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        // create lexer based on source kind
        mp_lexer_t *lex;
        if (source_kind == LEX_SRC_STR) {
            const char *line = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, line, strlen(line), false);
        } else if (source_kind == LEX_SRC_VSTR) {
            const vstr_t *vstr = source;
            lex = mp_lexer_new_from_str_len(MP_QSTR__lt_stdin_gt_, vstr->buf, vstr->len, false);
        } else if (source_kind == LEX_SRC_FILENAME) {
            lex = mp_lexer_new_from_file((const char *)source);
        } else { // LEX_SRC_STDIN
            lex = mp_lexer_new_from_fd(MP_QSTR__lt_stdin_gt_, 0, false);
        }

        qstr source_name = lex->source_name;

        // #if MICROPY_PY___FILE__
        // if (input_kind == MP_PARSE_FILE_INPUT) {
        //     mp_store_global(MP_QSTR___file__, MP_OBJ_NEW_QSTR(source_name));
        // }
        // #endif

        mp_parse_tree_t parse_tree = mp_parse(lex, input_kind);

        #if defined(MICROPY_UNIX_COVERAGE)
        // allow to print the parse tree in the coverage build
        if (mp_verbose_flag >= 3) {
            printf("----------------\n");
            mp_parse_node_print(&mp_plat_print, parse_tree.root, 0);
            printf("----------------\n");
        }
        #endif

        mp_obj_t module_fun = mp_compile(&parse_tree, source_name, is_repl);

        if (!compile_only) {
            // execute it
            mp_call_function_0(module_fun);
        }



        mp_hal_set_interrupt_char(-1);
        mp_handle_pending(true);
        nlr_pop();
        return 0;

    } else {
        // uncaught exception
        mp_hal_set_interrupt_char(-1);
        mp_handle_pending(false);
        return handle_uncaught_exception(nlr.ret_val);
    }
}

#if MICROPY_USE_READLINE == 1
#include "shared/readline/readline.h"
#else
STATIC char *strjoin(const char *s1, int sep_char, const char *s2) {
    int l1 = strlen(s1);
    int l2 = strlen(s2);
    char *s = malloc(l1 + l2 + 2);
    memcpy(s, s1, l1);
    if (sep_char != 0) {
        s[l1] = sep_char;
        l1 += 1;
    }
    memcpy(s + l1, s2, l2);
    s[l1 + l2] = 0;
    return s;
}
#endif

STATIC int do_repl(void) {
    mp_hal_stdout_tx_str(MICROPY_BANNER_NAME_AND_VERSION);
    mp_hal_stdout_tx_str("; " MICROPY_BANNER_MACHINE);
    mp_hal_stdout_tx_str("\nUse Ctrl-D to exit, Ctrl-E for paste mode\n");

    #if MICROPY_USE_READLINE == 1

    // use MicroPython supplied readline

    vstr_t line;
    vstr_init(&line, 16);
    for (;;) {
        mp_hal_stdio_mode_raw();

    input_restart:
        vstr_reset(&line);
        int ret = readline(&line, mp_repl_get_ps1());
        mp_parse_input_kind_t parse_input_kind = MP_PARSE_SINGLE_INPUT;

        if (ret == CHAR_CTRL_C) {
            // cancel input
            mp_hal_stdout_tx_str("\r\n");
            goto input_restart;
        } else if (ret == CHAR_CTRL_D) {
            // EOF
            printf("\n");
            mp_hal_stdio_mode_orig();
            vstr_clear(&line);
            return 0;
        } else if (ret == CHAR_CTRL_E) {
            // paste mode
            mp_hal_stdout_tx_str("\npaste mode; Ctrl-C to cancel, Ctrl-D to finish\n=== ");
            vstr_reset(&line);
            for (;;) {
                char c = mp_hal_stdin_rx_chr();
                if (c == CHAR_CTRL_C) {
                    // cancel everything
                    mp_hal_stdout_tx_str("\n");
                    goto input_restart;
                } else if (c == CHAR_CTRL_D) {
                    // end of input
                    mp_hal_stdout_tx_str("\n");
                    break;
                } else {
                    // add char to buffer and echo
                    vstr_add_byte(&line, c);
                    if (c == '\r') {
                        mp_hal_stdout_tx_str("\n=== ");
                    } else {
                        mp_hal_stdout_tx_strn(&c, 1);
                    }
                }
            }
            parse_input_kind = MP_PARSE_FILE_INPUT;
        } else if (line.len == 0) {
            if (ret != 0) {
                printf("\n");
            }
            goto input_restart;
        } else {
            // got a line with non-zero length, see if it needs continuing
            while (mp_repl_continue_with_input(vstr_null_terminated_str(&line))) {
                vstr_add_byte(&line, '\n');
                ret = readline(&line, mp_repl_get_ps2());
                if (ret == CHAR_CTRL_C) {
                    // cancel everything
                    printf("\n");
                    goto input_restart;
                } else if (ret == CHAR_CTRL_D) {
                    // stop entering compound statement
                    break;
                }
            }
        }

        mp_hal_stdio_mode_orig();

        ret = execute_from_lexer(LEX_SRC_VSTR, &line, parse_input_kind, true);
        if (ret & FORCED_EXIT) {
            return ret;
        }
    }

    #else

    // use simple readline

    for (;;) {
        char *line = prompt((char *)mp_repl_get_ps1());
        if (line == NULL) {
            // EOF
            return 0;
        }
        while (mp_repl_continue_with_input(line)) {
            char *line2 = prompt((char *)mp_repl_get_ps2());
            if (line2 == NULL) {
                break;
            }
            char *line3 = strjoin(line, '\n', line2);
            free(line);
            free(line2);
            line = line3;
        }

        int ret = execute_from_lexer(LEX_SRC_STR, line, MP_PARSE_SINGLE_INPUT, true);
        if (ret & FORCED_EXIT) {
            return ret;
        }
        free(line);
    }

    #endif
}

STATIC int do_file_loop(const char *file) {
    unsigned long total_time = 0;
    for (int i = 0; i < 10; i++) {
        unsigned long start_time = ustime();
        execute_from_lexer(LEX_SRC_FILENAME, file, MP_PARSE_FILE_INPUT, false);
        unsigned long stop_time = ustime();
        printf("Run, %ld ns\n", (stop_time - start_time));
        total_time += (stop_time - start_time);
    }
    printf("Run, average over 10 %ld ns\n", total_time / 10);
    asm("hlt \n");
    return 0;
}

STATIC int do_file(const char *file) {
    return execute_from_lexer(LEX_SRC_FILENAME, file, MP_PARSE_FILE_INPUT, false);

}

STATIC int do_file_fork(const char *file) {
    return execute_from_lexer_modified(LEX_SRC_FILENAME, file, MP_PARSE_FILE_INPUT, false);

}

STATIC int do_str(const char *str) {
    return execute_from_lexer(LEX_SRC_STR, str, MP_PARSE_FILE_INPUT, false);
}

STATIC void print_help(char **argv) {
    printf(
        "usage: %s [<opts>] [-X <implopt>] [-c <command> | -m <module> | <filename>]\n"
        "Options:\n"
        "-h : print this help message\n"
        "-i : enable inspection via REPL after running command/module/file\n"
        #if MICROPY_DEBUG_PRINTERS
        "-v : verbose (trace various operations); can be multiple\n"
        #endif
        "-O[N] : apply bytecode optimizations of level N\n"
        "\n"
        "Implementation specific options (-X):\n", argv[0]
        );
    int impl_opts_cnt = 0;
    printf(
        "  compile-only                 -- parse and compile only\n"
        #if MICROPY_EMIT_NATIVE
        "  emit={bytecode,native,viper} -- set the default code emitter\n"
        #else
        "  emit=bytecode                -- set the default code emitter\n"
        #endif
        );
    impl_opts_cnt++;
    #if MICROPY_ENABLE_GC
    printf(
        "  heapsize=<n>[w][K|M] -- set the heap size for the GC (default %ld)\n"
        , heap_size);
    impl_opts_cnt++;
    #endif
    #if defined(__APPLE__)
    printf("  realtime -- set thread priority to realtime\n");
    impl_opts_cnt++;
    #endif

    if (impl_opts_cnt == 0) {
        printf("  (none)\n");
    }
}

STATIC int invalid_args(void) {
    fprintf(stderr, "Invalid command line arguments. Use -h option for help.\n");
    return 1;
}

// Process options which set interpreter init options
STATIC void pre_process_options(int argc, char **argv) {
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-') {
            if (strcmp(argv[a], "-c") == 0 || strcmp(argv[a], "-m") == 0) {
                break; // Everything after this is a command/module and arguments for it
            }
            if (strcmp(argv[a], "-h") == 0) {
                print_help(argv);
                exit(0);
            }
            if (strcmp(argv[a], "-X") == 0) {
                if (a + 1 >= argc) {
                    exit(invalid_args());
                }
                if (0) {
                } else if (strcmp(argv[a + 1], "compile-only") == 0) {
                    compile_only = true;
                } else if (strcmp(argv[a + 1], "emit=bytecode") == 0) {
                    emit_opt = MP_EMIT_OPT_BYTECODE;
                #if MICROPY_EMIT_NATIVE
                } else if (strcmp(argv[a + 1], "emit=native") == 0) {
                    emit_opt = MP_EMIT_OPT_NATIVE_PYTHON;
                } else if (strcmp(argv[a + 1], "emit=viper") == 0) {
                    emit_opt = MP_EMIT_OPT_VIPER;
                #endif
                #if MICROPY_ENABLE_GC
                } else if (strncmp(argv[a + 1], "heapsize=", sizeof("heapsize=") - 1) == 0) {
                    char *end;
                    heap_size = strtol(argv[a + 1] + sizeof("heapsize=") - 1, &end, 0);
                    // Don't bring unneeded libc dependencies like tolower()
                    // If there's 'w' immediately after number, adjust it for
                    // target word size. Note that it should be *before* size
                    // suffix like K or M, to avoid confusion with kilowords,
                    // etc. the size is still in bytes, just can be adjusted
                    // for word size (taking 32bit as baseline).
                    bool word_adjust = false;
                    if ((*end | 0x20) == 'w') {
                        word_adjust = true;
                        end++;
                    }
                    if ((*end | 0x20) == 'k') {
                        heap_size *= 1024;
                    } else if ((*end | 0x20) == 'm') {
                        heap_size *= 1024 * 1024;
                    } else {
                        // Compensate for ++ below
                        --end;
                    }
                    if (*++end != 0) {
                        goto invalid_arg;
                    }
                    if (word_adjust) {
                        heap_size = heap_size * MP_BYTES_PER_OBJ_WORD / 4;
                    }
                    // If requested size too small, we'll crash anyway
                    if (heap_size < 700) {
                        goto invalid_arg;
                    }
                #endif
                #if defined(__APPLE__)
                } else if (strcmp(argv[a + 1], "realtime") == 0) {
                    #if MICROPY_PY_THREAD
                    mp_thread_is_realtime_enabled = true;
                    #endif
                    // main thread was already initialized before the option
                    // was parsed, so we have to enable realtime here.
                    mp_thread_set_realtime();
                #endif
                } else {
                invalid_arg:
                    exit(invalid_args());
                }
                a++;
            }
        } else {
            break; // Not an option but a file
        }
    }
}

STATIC void set_sys_argv(char *argv[], int argc, int start_arg) {
    for (int i = start_arg; i < argc; i++) {
        mp_obj_list_append(mp_sys_argv, MP_OBJ_NEW_QSTR(qstr_from_str(argv[i])));
    }
}

#if MICROPY_PY_SYS_EXECUTABLE
extern mp_obj_str_t mp_sys_executable_obj;
STATIC char executable_path[MICROPY_ALLOC_PATH_MAX];

STATIC void sys_set_excecutable(char *argv0) {
    if (realpath(argv0, executable_path)) {
        mp_obj_str_set_data(&mp_sys_executable_obj, (byte *)executable_path, strlen(executable_path));
    }
}
#endif

#ifdef _WIN32
#define PATHLIST_SEP_CHAR ';'
#else
#define PATHLIST_SEP_CHAR ':'
#endif

unsigned char helloworld_py[] = {
  0x64, 0x65, 0x66, 0x20, 0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x3a, 0x0a,
  0x20, 0x20, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x28, 0x27, 0x48, 0x65, 0x6c,
  0x6c, 0x6f, 0x2c, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x27, 0x29,
  0x0a, 0x0a, 0x69, 0x66, 0x20, 0x5f, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x5f,
  0x5f, 0x20, 0x3d, 0x3d, 0x20, 0x22, 0x5f, 0x5f, 0x6d, 0x61, 0x69, 0x6e,
  0x5f, 0x5f, 0x22, 0x3a, 0x0a, 0x20, 0x20, 0x6d, 0x61, 0x69, 0x6e, 0x28,
  0x29, 0x0a
};
unsigned int helloworld_py_len = 74;
unsigned char float_operation_py[] = {
  0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x6d, 0x61, 0x74, 0x68, 0x0a,
  0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x70, 0x79, 0x61, 0x65, 0x73,
  0x0a, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x68, 0x65, 0x6c, 0x6c,
  0x6f, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x0a, 0x0a, 0x0a, 0x64, 0x65, 0x66,
  0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x5f, 0x6f, 0x70, 0x65, 0x72, 0x61,
  0x74, 0x69, 0x6f, 0x6e, 0x73, 0x28, 0x6e, 0x29, 0x3a, 0x0a, 0x20, 0x20,
  0x20, 0x20, 0x73, 0x74, 0x61, 0x72, 0x74, 0x20, 0x3d, 0x20, 0x30, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x69, 0x20, 0x69, 0x6e,
  0x20, 0x72, 0x61, 0x6e, 0x67, 0x65, 0x28, 0x30, 0x2c, 0x20, 0x6e, 0x29,
  0x3a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x23, 0x73,
  0x69, 0x6e, 0x5f, 0x69, 0x20, 0x3d, 0x20, 0x6d, 0x61, 0x74, 0x68, 0x2e,
  0x73, 0x69, 0x6e, 0x28, 0x69, 0x29, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x23, 0x63, 0x6f, 0x73, 0x5f, 0x69, 0x20, 0x3d, 0x20,
  0x6d, 0x61, 0x74, 0x68, 0x2e, 0x63, 0x6f, 0x73, 0x28, 0x69, 0x29, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x23, 0x73, 0x71, 0x72,
  0x74, 0x5f, 0x69, 0x20, 0x3d, 0x20, 0x6d, 0x61, 0x74, 0x68, 0x2e, 0x73,
  0x71, 0x72, 0x74, 0x28, 0x69, 0x29, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x61, 0x64, 0x64, 0x65, 0x64, 0x20, 0x3d, 0x20, 0x69,
  0x20, 0x2b, 0x20, 0x31, 0x0a, 0x0a, 0x0a, 0x64, 0x65, 0x66, 0x20, 0x6d,
  0x61, 0x69, 0x6e, 0x28, 0x29, 0x3a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x23,
  0x6c, 0x61, 0x74, 0x65, 0x6e, 0x63, 0x69, 0x65, 0x73, 0x20, 0x3d, 0x20,
  0x7b, 0x7d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x23, 0x74, 0x69, 0x6d, 0x65,
  0x73, 0x74, 0x61, 0x6d, 0x70, 0x73, 0x20, 0x3d, 0x20, 0x7b, 0x7d, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x23, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x61,
  0x6d, 0x70, 0x73, 0x5b, 0x22, 0x73, 0x74, 0x61, 0x72, 0x74, 0x69, 0x6e,
  0x67, 0x5f, 0x74, 0x69, 0x6d, 0x65, 0x22, 0x5d, 0x20, 0x3d, 0x20, 0x30,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x23, 0x6e, 0x20, 0x3d, 0x20, 0x69, 0x6e,
  0x74, 0x28, 0x65, 0x76, 0x65, 0x6e, 0x74, 0x5b, 0x27, 0x6e, 0x27, 0x5d,
  0x29, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x6e, 0x20, 0x3d, 0x20, 0x31, 0x30,
  0x30, 0x30, 0x30, 0x30, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x6c, 0x61, 0x74,
  0x65, 0x6e, 0x63, 0x79, 0x20, 0x3d, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74,
  0x5f, 0x6f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x28,
  0x6e, 0x29, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x23, 0x6c, 0x61, 0x74, 0x65,
  0x6e, 0x63, 0x69, 0x65, 0x73, 0x5b, 0x22, 0x66, 0x75, 0x6e, 0x63, 0x74,
  0x69, 0x6f, 0x6e, 0x5f, 0x65, 0x78, 0x65, 0x63, 0x75, 0x74, 0x69, 0x6f,
  0x6e, 0x22, 0x5d, 0x20, 0x3d, 0x20, 0x6c, 0x61, 0x74, 0x65, 0x6e, 0x63,
  0x79, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x23, 0x74, 0x69, 0x6d, 0x65, 0x73,
  0x74, 0x61, 0x6d, 0x70, 0x73, 0x5b, 0x22, 0x66, 0x69, 0x6e, 0x69, 0x73,
  0x68, 0x69, 0x6e, 0x67, 0x5f, 0x74, 0x69, 0x6d, 0x65, 0x22, 0x5d, 0x20,
  0x3d, 0x20, 0x31, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x23, 0x72, 0x65, 0x74,
  0x75, 0x72, 0x6e, 0x20, 0x7b, 0x22, 0x6c, 0x61, 0x74, 0x65, 0x6e, 0x63,
  0x69, 0x65, 0x73, 0x22, 0x3a, 0x20, 0x6c, 0x61, 0x74, 0x65, 0x6e, 0x63,
  0x69, 0x65, 0x73, 0x2c, 0x20, 0x22, 0x74, 0x69, 0x6d, 0x65, 0x73, 0x74,
  0x61, 0x6d, 0x70, 0x73, 0x22, 0x3a, 0x20, 0x74, 0x69, 0x6d, 0x65, 0x73,
  0x74, 0x61, 0x6d, 0x70, 0x73, 0x7d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x23,
  0x70, 0x72, 0x69, 0x6e, 0x74, 0x28, 0x22, 0x64, 0x6f, 0x6e, 0x65, 0x22,
  0x29, 0x0a, 0x0a, 0x69, 0x66, 0x20, 0x5f, 0x5f, 0x6e, 0x61, 0x6d, 0x65,
  0x5f, 0x5f, 0x20, 0x3d, 0x3d, 0x20, 0x22, 0x5f, 0x5f, 0x6d, 0x61, 0x69,
  0x6e, 0x5f, 0x5f, 0x22, 0x3a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x6d, 0x61,
  0x69, 0x6e, 0x28, 0x29, 0x0a
};
unsigned int float_operation_py_len = 593;

unsigned char pyaes_py[] = {
  0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x72, 0x61, 0x6e, 0x64, 0x6f,
  0x6d, 0x0a, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74, 0x20, 0x63, 0x72, 0x79,
  0x70, 0x74, 0x6f, 0x6c, 0x69, 0x62, 0x0a, 0x69, 0x6d, 0x70, 0x6f, 0x72,
  0x74, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x5f, 0x6f, 0x70, 0x65, 0x72,
  0x61, 0x74, 0x69, 0x6f, 0x6e, 0x0a, 0x69, 0x6d, 0x70, 0x6f, 0x72, 0x74,
  0x20, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x77, 0x6f, 0x72, 0x6c, 0x64, 0x0a,
  0x0a, 0x0a, 0x64, 0x65, 0x66, 0x20, 0x67, 0x65, 0x6e, 0x65, 0x72, 0x61,
  0x74, 0x65, 0x28, 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x29, 0x3a, 0x0a,
  0x20, 0x20, 0x20, 0x20, 0x6c, 0x65, 0x74, 0x74, 0x65, 0x72, 0x73, 0x20,
  0x3d, 0x20, 0x22, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
  0x76, 0x77, 0x78, 0x79, 0x7a, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x30, 0x22, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x72, 0x65, 0x74,
  0x75, 0x72, 0x6e, 0x20, 0x27, 0x27, 0x2e, 0x6a, 0x6f, 0x69, 0x6e, 0x28,
  0x72, 0x61, 0x6e, 0x64, 0x6f, 0x6d, 0x2e, 0x63, 0x68, 0x6f, 0x69, 0x63,
  0x65, 0x28, 0x6c, 0x65, 0x74, 0x74, 0x65, 0x72, 0x73, 0x29, 0x20, 0x66,
  0x6f, 0x72, 0x20, 0x69, 0x20, 0x69, 0x6e, 0x20, 0x72, 0x61, 0x6e, 0x67,
  0x65, 0x28, 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x29, 0x29, 0x0a, 0x0a,
  0x0a, 0x64, 0x65, 0x66, 0x20, 0x6d, 0x61, 0x69, 0x6e, 0x28, 0x29, 0x3a,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x6c, 0x61, 0x74, 0x65, 0x6e, 0x63, 0x69,
  0x65, 0x73, 0x20, 0x3d, 0x20, 0x7b, 0x7d, 0x0a, 0x20, 0x20, 0x20, 0x20,
  0x74, 0x69, 0x6d, 0x65, 0x73, 0x74, 0x61, 0x6d, 0x70, 0x73, 0x20, 0x3d,
  0x20, 0x7b, 0x7d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x5f, 0x6f, 0x66, 0x5f, 0x6d,
  0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x3d, 0x20, 0x31, 0x30, 0x30,
  0x30, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x6e, 0x75, 0x6d, 0x5f, 0x6f, 0x66,
  0x5f, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x20,
  0x3d, 0x20, 0x31, 0x30, 0x30, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x6d, 0x65,
  0x73, 0x73, 0x61, 0x67, 0x65, 0x20, 0x3d, 0x20, 0x67, 0x65, 0x6e, 0x65,
  0x72, 0x61, 0x74, 0x65, 0x28, 0x6c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x5f,
  0x6f, 0x66, 0x5f, 0x6d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x29, 0x0a,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x23, 0x20, 0x31, 0x32, 0x38, 0x2d, 0x62,
  0x69, 0x74, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x28, 0x31, 0x36, 0x20, 0x62,
  0x79, 0x74, 0x65, 0x73, 0x29, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x4b, 0x45,
  0x59, 0x20, 0x3d, 0x20, 0x62, 0x27, 0x5c, 0x78, 0x61, 0x31, 0x5c, 0x78,
  0x66, 0x36, 0x25, 0x5c, 0x78, 0x38, 0x63, 0x5c, 0x78, 0x38, 0x37, 0x7d,
  0x5f, 0x5c, 0x78, 0x63, 0x64, 0x5c, 0x78, 0x38, 0x39, 0x64, 0x48, 0x45,
  0x38, 0x5c, 0x78, 0x62, 0x66, 0x5c, 0x78, 0x63, 0x39, 0x2c, 0x27, 0x0a,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6f, 0x72, 0x20, 0x6c, 0x6f, 0x6f,
  0x70, 0x73, 0x20, 0x69, 0x6e, 0x20, 0x72, 0x61, 0x6e, 0x67, 0x65, 0x28,
  0x6e, 0x75, 0x6d, 0x5f, 0x6f, 0x66, 0x5f, 0x69, 0x74, 0x65, 0x72, 0x61,
  0x74, 0x69, 0x6f, 0x6e, 0x73, 0x29, 0x3a, 0x0a, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x61, 0x65, 0x73, 0x20, 0x3d, 0x20, 0x63, 0x72,
  0x79, 0x70, 0x74, 0x6f, 0x6c, 0x69, 0x62, 0x2e, 0x61, 0x65, 0x73, 0x28,
  0x4b, 0x45, 0x59, 0x2c, 0x20, 0x36, 0x2c, 0x20, 0x4b, 0x45, 0x59, 0x29,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x69, 0x70,
  0x68, 0x65, 0x72, 0x74, 0x65, 0x78, 0x74, 0x20, 0x3d, 0x20, 0x61, 0x65,
  0x73, 0x2e, 0x65, 0x6e, 0x63, 0x72, 0x79, 0x70, 0x74, 0x28, 0x6d, 0x65,
  0x73, 0x73, 0x61, 0x67, 0x65, 0x29, 0x0a, 0x0a, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x61, 0x65, 0x73, 0x20, 0x3d, 0x20, 0x63, 0x72,
  0x79, 0x70, 0x74, 0x6f, 0x6c, 0x69, 0x62, 0x2e, 0x61, 0x65, 0x73, 0x28,
  0x4b, 0x45, 0x59, 0x2c, 0x20, 0x36, 0x2c, 0x20, 0x4b, 0x45, 0x59, 0x29,
  0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0x6c, 0x61,
  0x69, 0x6e, 0x74, 0x65, 0x78, 0x74, 0x20, 0x3d, 0x20, 0x61, 0x65, 0x73,
  0x2e, 0x64, 0x65, 0x63, 0x72, 0x79, 0x70, 0x74, 0x28, 0x63, 0x69, 0x70,
  0x68, 0x65, 0x72, 0x74, 0x65, 0x78, 0x74, 0x29, 0x0a, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x61, 0x65, 0x73, 0x20, 0x3d, 0x20, 0x4e,
  0x6f, 0x6e, 0x65, 0x0a, 0x0a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x70, 0x72,
  0x69, 0x6e, 0x74, 0x20, 0x28, 0x22, 0x44, 0x6f, 0x6e, 0x65, 0x22, 0x29,
  0x0a, 0x0a, 0x69, 0x66, 0x20, 0x5f, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x5f,
  0x5f, 0x20, 0x3d, 0x3d, 0x20, 0x22, 0x5f, 0x5f, 0x6d, 0x61, 0x69, 0x6e,
  0x5f, 0x5f, 0x22, 0x3a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x6d, 0x61, 0x69,
  0x6e, 0x28, 0x29, 0x0a
};
unsigned int pyaes_py_len = 748;

MP_NOINLINE int main_(int argc, char **argv);

int main(int argc, char **argv) {
     int fd = open("helloworld.py", O_RDWR|O_CREAT, 0777);
 	write(fd, helloworld_py, helloworld_py_len);
    fd = open("float_operation.py", O_RDWR|O_CREAT, 0777);
 	write(fd, float_operation_py, float_operation_py_len);
    fd = open("pyaes.py", O_RDWR|O_CREAT, 0777);
 	write(fd, pyaes_py, pyaes_py_len);
    #if MICROPY_PY_THREAD
    mp_thread_init();
    #endif
    // We should capture stack top ASAP after start, and it should be
    // captured guaranteedly before any other stack variables are allocated.
    // For this, actual main (renamed main_) should not be inlined into
    // this function. main_() itself may have other functions inlined (with
    // their own stack variables), that's why we need this main/main_ split.
    mp_stack_ctrl_init();
    return main_(argc, argv);
}

MP_NOINLINE int main_(int argc, char **argv) {
    #ifdef SIGPIPE
    // Do not raise SIGPIPE, instead return EPIPE. Otherwise, e.g. writing
    // to peer-closed socket will lead to sudden termination of MicroPython
    // process. SIGPIPE is particularly nasty, because unix shell doesn't
    // print anything for it, so the above looks like completely sudden and
    // silent termination for unknown reason. Ignoring SIGPIPE is also what
    // CPython does. Note that this may lead to problems using MicroPython
    // scripts as pipe filters, but again, that's what CPython does. So,
    // scripts which want to follow unix shell pipe semantics (where SIGPIPE
    // means "pipe was requested to terminate, it's not an error"), should
    // catch EPIPE themselves.
    signal(SIGPIPE, SIG_IGN);
    #endif
    

    // Define a reasonable stack limit to detect stack overflow.
    mp_uint_t stack_limit = 40000 * (sizeof(void *) / 4);
    #if defined(__arm__) && !defined(__thumb2__)
    // ARM (non-Thumb) architectures require more stack.
    stack_limit *= 2;
    #endif
    mp_stack_set_limit(stack_limit);

    pre_process_options(argc, argv);

    #if MICROPY_ENABLE_GC
    #if !MICROPY_GC_SPLIT_HEAP
    char *heap = malloc(heap_size);
    gc_init(heap, heap + heap_size);
    #else
    //assert(MICROPY_GC_SPLIT_HEAP_N_HEAPS > 0);
    char *heaps[MICROPY_GC_SPLIT_HEAP_N_HEAPS];
    long multi_heap_size = heap_size / MICROPY_GC_SPLIT_HEAP_N_HEAPS;
    for (size_t i = 0; i < MICROPY_GC_SPLIT_HEAP_N_HEAPS; i++) {
        heaps[i] = malloc(multi_heap_size);
        if (i == 0) {
            gc_init(heaps[i], heaps[i] + multi_heap_size);
        } else {
            gc_add(heaps[i], heaps[i] + multi_heap_size);
        }
    }
    #endif
    #endif

    #if MICROPY_ENABLE_PYSTACK
    static mp_obj_t pystack[1024];
    mp_pystack_init(pystack, &pystack[MP_ARRAY_SIZE(pystack)]);
    #endif

    mp_init();

    #if MICROPY_EMIT_NATIVE
    // Set default emitter options
    MP_STATE_VM(default_emit_opt) = emit_opt;
    #else
    (void)emit_opt;
    #endif

    #if MICROPY_VFS_POSIX
    {
        // Mount the host FS at the root of our internal VFS
        mp_obj_t args[2] = {
            MP_OBJ_TYPE_GET_SLOT(&mp_type_vfs_posix, make_new)(&mp_type_vfs_posix, 0, 0, NULL),
            MP_OBJ_NEW_QSTR(MP_QSTR__slash_),
        };
        mp_vfs_mount(2, args, (mp_map_t *)&mp_const_empty_map);
        MP_STATE_VM(vfs_cur) = MP_STATE_VM(vfs_mount_table);
    }
    #endif
    {
        // sys.path starts as [""]
        mp_sys_path = mp_obj_new_list(0, NULL);
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));

        // Add colon-separated entries from MICROPYPATH.
        char *home = getenv("HOME");
        char *path = getenv("MICROPYPATH");
        if (path == NULL) {
            path = MICROPY_PY_SYS_PATH_DEFAULT;
        }
        if (*path == PATHLIST_SEP_CHAR) {
            // First entry is empty. We've already added an empty entry to sys.path, so skip it.
            ++path;
        }
        bool path_remaining = *path;
        while (path_remaining) {
            char *path_entry_end = strchr(path, PATHLIST_SEP_CHAR);
            if (path_entry_end == NULL) {
                path_entry_end = path + strlen(path);
                path_remaining = false;
            }
            if (path[0] == '~' && path[1] == '/' && home != NULL) {
                // Expand standalone ~ to $HOME
                int home_l = strlen(home);
                vstr_t vstr;
                vstr_init(&vstr, home_l + (path_entry_end - path - 1) + 1);
                vstr_add_strn(&vstr, home, home_l);
                vstr_add_strn(&vstr, path + 1, path_entry_end - path - 1);
                mp_obj_list_append(mp_sys_path, mp_obj_new_str_from_vstr(&vstr));
            } else {
                mp_obj_list_append(mp_sys_path, mp_obj_new_str_via_qstr(path, path_entry_end - path));
            }
            path = path_entry_end + 1;
        }
    }

    mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

    #if defined(MICROPY_UNIX_COVERAGE)
    {
        MP_DECLARE_CONST_FUN_OBJ_0(extra_coverage_obj);
        MP_DECLARE_CONST_FUN_OBJ_0(extra_cpp_coverage_obj);
        mp_store_global(MP_QSTR_extra_coverage, MP_OBJ_FROM_PTR(&extra_coverage_obj));
        mp_store_global(MP_QSTR_extra_cpp_coverage, MP_OBJ_FROM_PTR(&extra_cpp_coverage_obj));
    }
    #endif

    // Here is some example code to create a class and instance of that class.
    // First is the Python, then the C code.
    //
    // class TestClass:
    //     pass
    // test_obj = TestClass()
    // test_obj.attr = 42
    //
    // mp_obj_t test_class_type, test_class_instance;
    // test_class_type = mp_obj_new_type(qstr_from_str("TestClass"), mp_const_empty_tuple, mp_obj_new_dict(0));
    // mp_store_name(qstr_from_str("test_obj"), test_class_instance = mp_call_function_0(test_class_type));
    // mp_store_attr(test_class_instance, qstr_from_str("attr"), mp_obj_new_int(42));

    /*
    printf("bytes:\n");
    printf("    total %d\n", m_get_total_bytes_allocated());
    printf("    cur   %d\n", m_get_current_bytes_allocated());
    printf("    peak  %d\n", m_get_peak_bytes_allocated());
    */

    #if MICROPY_PY_SYS_EXECUTABLE
    sys_set_excecutable(argv[0]);
    #endif

    const int NOTHING_EXECUTED = -2;
    int ret = NOTHING_EXECUTED;
    bool inspect = false;
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-') {
            if (strcmp(argv[a], "-i") == 0) {
                inspect = true;
            } else if (strcmp(argv[a], "-c") == 0) {
                if (a + 1 >= argc) {
                    return invalid_args();
                }
                set_sys_argv(argv, a + 1, a); // The -c becomes first item of sys.argv, as in CPython
                set_sys_argv(argv, argc, a + 2); // Then what comes after the command
                ret = do_str(argv[a + 1]);
                break;
            } else if (strcmp(argv[a], "-m") == 0) {
                if (a + 1 >= argc) {
                    return invalid_args();
                }
                mp_obj_t import_args[4];
                import_args[0] = mp_obj_new_str(argv[a + 1], strlen(argv[a + 1]));
                import_args[1] = import_args[2] = mp_const_none;
                // Ask __import__ to handle imported module specially - set its __name__
                // to __main__, and also return this leaf module, not top-level package
                // containing it.
                import_args[3] = mp_const_false;
                // TODO: https://docs.python.org/3/using/cmdline.html#cmdoption-m :
                // "the first element of sys.argv will be the full path to
                // the module file (while the module file is being located,
                // the first element will be set to "-m")."
                set_sys_argv(argv, argc, a + 1);

                mp_obj_t mod;
                nlr_buf_t nlr;

                // Allocating subpkg_tried on the stack can lead to compiler warnings about this
                // variable being clobbered when nlr is implemented using setjmp/longjmp.  Its
                // value must be preserved across calls to setjmp/longjmp.
                static bool subpkg_tried;
                subpkg_tried = false;

            reimport:
                if (nlr_push(&nlr) == 0) {
                    mod = mp_builtin___import__(MP_ARRAY_SIZE(import_args), import_args);
                    nlr_pop();
                } else {
                    // uncaught exception
                    return handle_uncaught_exception(nlr.ret_val) & 0xff;
                }

                // If this module is a package, see if it has a `__main__.py`.
                mp_obj_t dest[2];
                mp_load_method_protected(mod, MP_QSTR___path__, dest, true);
                if (dest[0] != MP_OBJ_NULL && !subpkg_tried) {
                    subpkg_tried = true;
                    vstr_t vstr;
                    int len = strlen(argv[a + 1]);
                    vstr_init(&vstr, len + sizeof(".__main__"));
                    vstr_add_strn(&vstr, argv[a + 1], len);
                    vstr_add_strn(&vstr, ".__main__", sizeof(".__main__") - 1);
                    import_args[0] = mp_obj_new_str_from_vstr(&vstr);
                    goto reimport;
                }

                ret = 0;
                break;
            } else if (strcmp(argv[a], "-X") == 0) {
                a += 1;
            #if MICROPY_DEBUG_PRINTERS
            } else if (strcmp(argv[a], "-v") == 0) {
                mp_verbose_flag++;
            #endif
            } else if (strncmp(argv[a], "-O", 2) == 0) {
                if (unichar_isdigit(argv[a][2])) {
                    MP_STATE_VM(mp_optimise_value) = argv[a][2] & 0xf;
                } else {
                    MP_STATE_VM(mp_optimise_value) = 0;
                    for (char *p = argv[a] + 1; *p && *p == 'O'; p++, MP_STATE_VM(mp_optimise_value)++) {;
                    }
                }
            } else {
                return invalid_args();
            }
        } else {
            char *pathbuf = malloc(PATH_MAX);
            char *basedir = realpath(argv[a], pathbuf);
            if (basedir == NULL) {
                mp_printf(&mp_stderr_print, "%s: can't open file '%s': [Errno %d] %s\n", argv[0], argv[a], errno, sfork_syscalls.strerror(errno));
                free(pathbuf);
                // CPython exits with 2 in such case
                ret = 2;
                break;
            }

            // Set base dir of the script as first entry in sys.path.
            char *p = strrchr(basedir, '/');
            mp_obj_list_store(mp_sys_path, MP_OBJ_NEW_SMALL_INT(0), mp_obj_new_str_via_qstr(basedir, p - basedir));
            free(pathbuf);

            set_sys_argv(argv, argc, a);
            ret = do_file(argv[a]);
            break;
        }
    }

    const char *inspect_env = getenv("MICROPYINSPECT");
    if (inspect_env && inspect_env[0] != '\0') {
        inspect = true;
    }
    /* see if script is available from initrd */
    struct ukplat_memregion_desc *img;
    char *cstr;
    
    //main_start = ustime();
    ret = do_file_fork("float_operation.py");
    //main_stop = ustime();
    sfork_syscalls.printf("Run fork, %ld runs, in %ld ns\n", executions, RUNTIME);
    
    /* repl mode */
    // else {
    //   #if MICROPY_REPL_EVENT_DRIVEN
    //   pyexec_event_repl_init();
    //   for (;;) {
	// int c = mp_hal_stdin_rx_chr();
	// if (pyexec_event_repl_process_char(c)) {
	//   break;
	// }
    //   }
    //   #else
    //   pyexec_friendly_repl();
    //   #endif
    //  }

    #if MICROPY_PY_SYS_SETTRACE
    MP_STATE_THREAD(prof_trace_callback) = MP_OBJ_NULL;
    #endif

    #if MICROPY_PY_SYS_ATEXIT
    // Beware, the sys.settrace callback should be disabled before running sys.atexit.
    if (mp_obj_is_callable(MP_STATE_VM(sys_exitfunc))) {
        mp_call_function_0(MP_STATE_VM(sys_exitfunc));
    }
    #endif

    #if MICROPY_PY_MICROPYTHON_MEM_INFO
    if (mp_verbose_flag) {
        mp_micropython_mem_info(0, NULL);
    }
    #endif

    #if MICROPY_PY_BLUETOOTH
    void mp_bluetooth_deinit(void);
    mp_bluetooth_deinit();
    #endif

    #if MICROPY_PY_THREAD
    mp_thread_deinit();
    #endif

    #if defined(MICROPY_UNIX_COVERAGE)
    gc_sweep_all();
    #endif

    mp_deinit();

    #if MICROPY_ENABLE_GC && !defined(NDEBUG)
    // We don't really need to free memory since we are about to exit the
    // process, but doing so helps to find memory leaks.
    #if !MICROPY_GC_SPLIT_HEAP
    free(heap);
    #else
    for (size_t i = 0; i < MICROPY_GC_SPLIT_HEAP_N_HEAPS; i++) {
        free(heaps[i]);
    }
    #endif
    #endif

    // printf("total bytes = %d\n", m_get_total_bytes_allocated());
    return ret & 0xff;
}

void nlr_jump_fail(void *val) {
    #if MICROPY_USE_READLINE == 1
    mp_hal_stdio_mode_orig();
    #endif
    fprintf(stderr, "FATAL: uncaught NLR %p\n", val);
    exit(1);
}

//    mp_import_stat_t mp_import_stat(const char *path) {
//     return MP_IMPORT_STAT_NO_EXIST;
// }
