
#include <ngx_config.h>
#include <ngx_core.h>

extern ngx_module_t  ngx_core_module;
extern ngx_module_t  ngx_errlog_module;
extern ngx_module_t  ngx_conf_module;
extern ngx_module_t  ngx_events_module;
extern ngx_module_t  ngx_event_core_module;
extern ngx_module_t  ngx_select_module;

#ifdef CONFIG_LIBNGINX_HTTP
#include <ngx_http.h>

extern ngx_module_t  ngx_http_module;
extern ngx_module_t  ngx_http_core_module;
extern ngx_module_t  ngx_http_log_module;

#ifdef CONFIG_LIBNGINX_UPSTREAM
extern ngx_module_t  ngx_http_upstream_module;
#endif

extern ngx_module_t  ngx_http_write_filter_module;
extern ngx_module_t  ngx_http_header_filter_module;
extern ngx_module_t  ngx_http_chunked_filter_module;

#ifdef CONFIG_LIBNGINX_HTTP_V2
extern ngx_module_t  ngx_http_v2_filter_module;
#endif

extern ngx_module_t  ngx_http_range_header_filter_module;

#ifdef CONFIG_LIBNGINX_HTTP_GZIP
extern ngx_module_t  ngx_http_gzip_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_POSTPONE
extern ngx_module_t  ngx_http_postpone_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SSI
extern ngx_module_t  ngx_http_ssi_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_CHARSET
extern ngx_module_t  ngx_http_charset_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_XSLT
extern ngx_module_t  ngx_http_xslt_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_IMAGE_FILTER
extern ngx_module_t  ngx_http_image_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SUB
extern ngx_module_t  ngx_http_sub_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_ADDITION
extern ngx_module_t  ngx_http_addition_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GUNZIP
extern ngx_module_t  ngx_http_gunzip_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_USERID
extern ngx_module_t  ngx_http_userid_filter_module;
#endif

extern ngx_module_t  ngx_http_headers_filter_module;
extern ngx_module_t  ngx_http_copy_filter_module;
extern ngx_module_t  ngx_http_range_body_filter_module;
extern ngx_module_t  ngx_http_not_modified_filter_module;

#ifdef CONFIG_LIBNGINX_HTTP_SLICE
extern ngx_module_t  ngx_http_slice_filter_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_V2
extern ngx_module_t  ngx_http_v2_module;
#endif

extern ngx_module_t  ngx_http_static_module;

#ifdef CONFIG_LIBNGINX_HTTP_GZIP_STATIC
extern ngx_module_t  ngx_http_gzip_static_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_DAV
extern ngx_module_t  ngx_http_dav_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_AUTOINDEX
extern ngx_module_t  ngx_http_autoindex_module;
#endif

extern ngx_module_t  ngx_http_index_module;

#ifdef CONFIG_LIBNGINX_HTTP_RANDOM_INDEX
extern ngx_module_t  ngx_http_random_index_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MIRROR
extern ngx_module_t  ngx_http_mirror_module;
#endif

extern ngx_module_t  ngx_http_try_files_module;

#ifdef CONFIG_LIBNGINX_HTTP_AUTH_REQUEST
extern ngx_module_t  ngx_http_auth_request_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_AUTH_BASIC
extern ngx_module_t  ngx_http_auth_basic_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_ACCESS
extern ngx_module_t  ngx_http_access_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_LIMIT_CONN
extern ngx_module_t  ngx_http_limit_conn_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_LIMIT_REQ
extern ngx_module_t  ngx_http_limit_req_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REALIP
extern ngx_module_t  ngx_http_realip_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_STATUS
extern ngx_module_t  ngx_http_status_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GEO
extern ngx_module_t  ngx_http_geo_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GEOIP
extern ngx_module_t  ngx_http_geoip_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MAP
extern ngx_module_t  ngx_http_map_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SPLIT_CLENTS
extern ngx_module_t  ngx_http_split_clients_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REFERER
extern ngx_module_t  ngx_http_referer_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REWRITE
extern ngx_module_t  ngx_http_rewrite_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SSL
extern ngx_module_t  ngx_http_ssl_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_PROXY
extern ngx_module_t  ngx_http_proxy_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_FASTCGI
extern ngx_module_t  ngx_http_fastcgi_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UWSGI
extern ngx_module_t  ngx_http_uwsgi_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SCGI
extern ngx_module_t  ngx_http_scgi_module;
#endif

#if defined CONFIG_LIBNGINX_HTTP_GRPC  && defined CONFIG_LIBNGINX_HTTP_V2
extern ngx_module_t  ngx_http_grpc_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_PERL
extern ngx_module_t  ngx_http_perl_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MEMCACHED
extern ngx_module_t  ngx_http_memcached_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_EMPTY_GIF
extern ngx_module_t  ngx_http_empty_gif_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_BROWSER
extern ngx_module_t  ngx_http_browser_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SECURE_LINK
extern ngx_module_t  ngx_http_secure_link_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_DEGRADATION
extern ngx_module_t  ngx_http_degradation_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_FLV
extern ngx_module_t  ngx_http_flv_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MP4
extern ngx_module_t  ngx_http_mp4_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_HASH
extern ngx_module_t  ngx_http_upstream_hash_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_IP_HASH
extern ngx_module_t  ngx_http_upstream_ip_hash_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_LEAST_CONN
extern ngx_module_t  ngx_http_upstream_least_conn_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_RANDOM
extern ngx_module_t  ngx_http_upstream_random_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_KEEPALIVE
extern ngx_module_t  ngx_http_upstream_keepalive_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_ZONE
extern ngx_module_t  ngx_http_upstream_zone_module;
#endif

#ifdef CONFIG_LIBNGINX_HTTP_STUB_STATUS
extern ngx_module_t  ngx_http_stub_status_module;
#endif
#endif

#ifdef CONFIG_LIBNGINX_MAIL
extern ngx_module_t  ngx_mail_module;
extern ngx_module_t  ngx_mail_core_module;

#ifdef CONFIG_LIBNGINX_MAIL_SSL
extern ngx_module_t  ngx_mail_ssl_module;
#endif

#ifdef CONFIG_LIBNGINX_MAIL_POP3
extern ngx_module_t  ngx_mail_pop3_module;
#endif

#ifdef CONFIG_LIBNGINX_MAIL_IMAP
extern ngx_module_t  ngx_mail_imap_module;
#endif

#ifdef CONFIG_LIBNGINX_MAIL_SMTP
extern ngx_module_t  ngx_mail_smtp_module;
#endif

extern ngx_module_t  ngx_mail_auth_http_module;
extern ngx_module_t  ngx_mail_proxy_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM
extern ngx_module_t  ngx_stream_module;
extern ngx_module_t  ngx_stream_core_module;
extern ngx_module_t  ngx_stream_log_module;
extern ngx_module_t  ngx_stream_proxy_module;

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM
extern ngx_module_t  ngx_stream_upstream_module;
#endif

extern ngx_module_t  ngx_stream_write_filter_module;

#ifdef CONFIG_LIBNGINX_STREAM_SSL
extern ngx_module_t  ngx_stream_ssl_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_REALIP
extern ngx_module_t  ngx_stream_realip_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_ACCESS
extern ngx_module_t  ngx_stream_access_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_GEO
extern ngx_module_t  ngx_stream_geo_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_GEOIP
extern ngx_module_t  ngx_stream_geoip_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_SPLIT_CLIENTS
extern ngx_module_t  ngx_stream_split_clients_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_RETURN
extern ngx_module_t  ngx_stream_return_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_LEAST_CONN
extern ngx_module_t  ngx_stream_upstream_least_conn_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_RANDOM
extern ngx_module_t  ngx_stream_upstream_random_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_ZONE
extern ngx_module_t  ngx_stream_upstream_zone_module;
#endif

#ifdef CONFIG_LIBNGINX_STREAM_SSL_PREREAD
extern ngx_module_t  ngx_stream_ssl_preread_module;
#endif
#endif

#ifdef CONFIG_LIBNSSL
extern ngx_module_t  ngx_openssl_module;
#endif

#ifdef CONFIG_LIBPCRE
extern ngx_module_t  ngx_regex_module;
#endif

ngx_module_t *ngx_modules[] = {
    &ngx_core_module,
    &ngx_errlog_module,
    &ngx_conf_module,
    &ngx_events_module,
    &ngx_event_core_module,
    &ngx_select_module,

#ifdef CONFIG_LIBNGINX_HTTP
    &ngx_http_module,
    &ngx_http_core_module,
    &ngx_http_log_module,

#ifdef CONFIG_LIBNGINX_UPSTREAM
    &ngx_http_upstream_module,
#endif

    &ngx_http_write_filter_module,
    &ngx_http_header_filter_module,
    &ngx_http_chunked_filter_module,

#ifdef CONFIG_LIBNGINX_HTTP_V2
    &ngx_http_v2_filter_module,
#endif

    &ngx_http_range_header_filter_module,

#ifdef CONFIG_LIBNGINX_HTTP_GZIP
    &ngx_http_gzip_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_POSTPONE
    &ngx_http_postpone_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SSI
    &ngx_http_ssi_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_CHARSET
    &ngx_http_charset_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_XSLT
    &ngx_http_xslt_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_IMAGE_FILTER
    &ngx_http_image_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SUB
    &ngx_http_sub_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_ADDITION
    &ngx_http_addition_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GUNZIP
    &ngx_http_gunzip_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_USERID
    &ngx_http_userid_filter_module,
#endif

    &ngx_http_headers_filter_module,
    &ngx_http_copy_filter_module,
    &ngx_http_range_body_filter_module,
    &ngx_http_not_modified_filter_module,

#ifdef CONFIG_LIBNGINX_HTTP_SLICE
    &ngx_http_slice_filter_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_V2
    &ngx_http_v2_module,
#endif

    &ngx_http_static_module,

#ifdef CONFIG_LIBNGINX_HTTP_GZIP_STATIC
    &ngx_http_gzip_static_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_DAV
    &ngx_http_dav_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_AUTOINDEX
    &ngx_http_autoindex_module,
#endif

    &ngx_http_index_module,

#ifdef CONFIG_LIBNGINX_HTTP_RANDOM_INDEX
    &ngx_http_random_index_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MIRROR
    &ngx_http_mirror_module,
#endif

    &ngx_http_try_files_module,

#ifdef CONFIG_LIBNGINX_HTTP_AUTH_REQUEST
    &ngx_http_auth_request_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_AUTH_BASIC
    &ngx_http_auth_basic_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_ACCESS
    &ngx_http_access_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_LIMIT_CONN
    &ngx_http_limit_conn_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_LIMIT_REQ
    &ngx_http_limit_req_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REALIP
    &ngx_http_realip_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_STATUS
    &ngx_http_status_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GEO
    &ngx_http_geo_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GEOIP
    &ngx_http_geoip_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MAP
    &ngx_http_map_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SPLIT_CLENTS
    &ngx_http_split_clients_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REFERER
    &ngx_http_referer_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REWRITE
    &ngx_http_rewrite_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SSL
    &ngx_http_ssl_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_PROXY
    &ngx_http_proxy_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_FASTCGI
    &ngx_http_fastcgi_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UWSGI
    &ngx_http_uwsgi_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SCGI
    &ngx_http_scgi_module,
#endif

#if defined CONFIG_LIBNGINX_HTTP_GRPC && defined CONFIG_LIBNGINX_HTTP_V2
    &ngx_http_grpc_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_PERL
    &ngx_http_perl_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MEMCACHED
    &ngx_http_memcached_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_EMPTY_GIF
    &ngx_http_empty_gif_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_BROWSER
    &ngx_http_browser_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SECURE_LINK
    &ngx_http_secure_link_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_DEGRADATION
    &ngx_http_degradation_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_FLV
    &ngx_http_flv_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MP4
    &ngx_http_mp4_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_HASH
    &ngx_http_upstream_hash_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_IP_HASH
    &ngx_http_upstream_ip_hash_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_LEAST_CONN
    &ngx_http_upstream_least_conn_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_RANDOM
    &ngx_http_upstream_random_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_KEEPALIVE
    &ngx_http_upstream_keepalive_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_ZONE
    &ngx_http_upstream_zone_module,
#endif

#ifdef CONFIG_LIBNGINX_HTTP_STUB_STATUS
    &ngx_http_stub_status_module,
#endif
#endif

#ifdef CONFIG_LIBNGINX_MAIL
    &ngx_mail_module,
    &ngx_mail_core_module,

#ifdef CONFIG_LIBNGINX_MAIL_SSL
    &ngx_mail_ssl_module,
#endif

#ifdef CONFIG_LIBNGINX_MAIL_POP3
    &ngx_mail_pop3_module,
#endif

#ifdef CONFIG_LIBNGINX_MAIL_IMAP
    &ngx_mail_imap_module,
#endif

#ifdef CONFIG_LIBNGINX_MAIL_SMTP
    &ngx_mail_smtp_module,
#endif

    &ngx_mail_auth_http_module,
    &ngx_mail_proxy_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM
    &ngx_stream_module,
    &ngx_stream_core_module,
    &ngx_stream_log_module,
    &ngx_stream_proxy_module,

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM
    &ngx_stream_upstream_module,
#endif

    &ngx_stream_write_filter_module,

#ifdef CONFIG_LIBNGINX_STREAM_SSL
    &ngx_stream_ssl_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_REALIP
    &ngx_stream_realip_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_ACCESS
    &ngx_stream_access_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_GEO
    &ngx_stream_geo_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_GEOIP
    &ngx_stream_geoip_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_SPLIT_CLIENTS
    &ngx_stream_split_clients_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_RETURN
    &ngx_stream_return_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_LEAST_CONN
    &ngx_stream_upstream_least_conn_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_RANDOM
    &ngx_stream_upstream_random_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_ZONE
    &ngx_stream_upstream_zone_module,
#endif

#ifdef CONFIG_LIBNGINX_STREAM_SSL_PREREAD
    &ngx_stream_ssl_preread_module,
#endif
#endif

#ifdef CONFIG_LIBNSSL
    &ngx_openssl_module,
#endif

#ifdef CONFIG_LIBPCRE
    &ngx_regex_module,
#endif

    NULL
};

char *ngx_module_names[] = {
    "ngx_core_module",
    "ngx_errlog_module",
    "ngx_conf_module",
    "ngx_events_module",
    "ngx_event_core_module",
    "ngx_select_module",

#ifdef CONFIG_LIBNGINX_HTTP
    "ngx_http_module",
    "ngx_http_core_module",
    "ngx_http_log_module",
    "ngx_http_write_filter_module",
    "ngx_http_header_filter_module",
    "ngx_http_chunked_filter_module",

#ifdef CONFIG_LIBNGINX_UPSTREAM
    "ngx_http_upstream_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_V2
    "ngx_http_v2_filter_module",
#endif

    "ngx_http_range_header_filter_module",

#ifdef CONFIG_LIBNGINX_HTTP_GZIP
    "ngx_http_gzip_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_POSTPONE
    "ngx_http_postpone_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SSI
    "ngx_http_ssi_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_CHARSET
    "ngx_http_charset_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_XSLT
    "ngx_http_xslt_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_IMAGE_FILTER
    "ngx_http_image_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SUB
    "ngx_http_sub_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_ADDITION
    "ngx_http_addition_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GUNZIP
    "ngx_http_gunzip_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_USERID
    "ngx_http_userid_filter_module",
#endif

    "ngx_http_headers_filter_module",
    "ngx_http_copy_filter_module",
    "ngx_http_range_body_filter_module",
    "ngx_http_not_modified_filter_module",

#ifdef CONFIG_LIBNGINX_HTTP_SLICE
    "ngx_http_slice_filter_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_V2
    "ngx_http_v2_module",
#endif

    "ngx_http_static_module",

#ifdef CONFIG_LIBNGINX_HTTP_GZIP_STATIC
    "ngx_http_gzip_static_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_DAV
    "ngx_http_dav_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_AUTOINDEX
    "ngx_http_autoindex_module",
#endif

    "ngx_http_index_module",

#ifdef CONFIG_LIBNGINX_HTTP_RANDOM_INDEX
    "ngx_http_random_index_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MIRROR
    "ngx_http_mirror_module",
#endif

    "ngx_http_try_files_module",

#ifdef CONFIG_LIBNGINX_HTTP_AUTH_REQUEST
    "ngx_http_auth_request_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_AUTH_BASIC
    "ngx_http_auth_basic_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_ACCESS
    "ngx_http_access_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_LIMIT_CONN
    "ngx_http_limit_conn_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_LIMIT_REQ
    "ngx_http_limit_req_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REALIP
    "ngx_http_realip_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_STATUS
    "ngx_http_status_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GEO
    "ngx_http_geo_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_GEOIP
    "ngx_http_geoip_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MAP
    "ngx_http_map_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SPLIT_CLENTS
    "ngx_http_split_clients_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REFERER
    "ngx_http_referer_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_REWRITE
    "ngx_http_rewrite_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SSL
    "ngx_http_ssl_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_PROXY
    "ngx_http_proxy_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_FASTCGI
    "ngx_http_fastcgi_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UWSGI
    "ngx_http_uwsgi_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SCGI
    "ngx_http_scgi_module",
#endif

#if defined CONFIG_LIBNGINX_HTTP_GRPC  && defined CONFIG_LIBNGINX_HTTP_V2
    "ngx_http_grpc_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_PERL
    "ngx_http_perl_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MEMCACHED
    "ngx_http_memcached_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_EMPTY_GIF
    "ngx_http_empty_gif_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_BROWSER
    "ngx_http_browser_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_SECURE_LINK
    "ngx_http_secure_link_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_DEGRADATION
    "ngx_http_degradation_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_FLV
    "ngx_http_flv_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_MP4
    "ngx_http_mp4_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_HASH
    "ngx_http_upstream_hash_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_IP_HASH
    "ngx_http_upstream_ip_hash_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_LEAST_CONN
    "ngx_http_upstream_least_conn_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_RANDOM
    "ngx_http_upstream_random_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_KEEPALIVE
    "ngx_http_upstream_keepalive_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_UPSTREAM_ZONE
    "ngx_http_upstream_zone_module",
#endif

#ifdef CONFIG_LIBNGINX_HTTP_STUB_STATUS
    "ngx_http_stub_status_module",
#endif
#endif

#ifdef CONFIG_LIBNGINX_MAIL
    "ngx_mail_module",
    "ngx_mail_core_module",

#ifdef CONFIG_LIBNGINX_MAIL_SSL
    "ngx_mail_ssl_module",
#endif

#ifdef CONFIG_LIBNGINX_MAIL_POP3
    "ngx_mail_pop3_module",
#endif

#ifdef CONFIG_LIBNGINX_MAIL_IMAP
    "ngx_mail_imap_module",
#endif

#ifdef CONFIG_LIBNGINX_MAIL_SMTP
    "ngx_mail_smtp_module",
#endif

    "ngx_mail_auth_http_module",
    "ngx_mail_proxy_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM
    "ngx_stream_module",
    "ngx_stream_core_module",
    "ngx_stream_log_module",
    "ngx_stream_proxy_module",

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM
    "ngx_stream_upstream_module",
#endif

    "ngx_stream_write_filter_module",

#ifdef CONFIG_LIBNGINX_STREAM_SSL
    "ngx_stream_ssl_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_REALIP
    "ngx_stream_realip_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_ACCESS
    "ngx_stream_access_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_GEO
    "ngx_stream_geo_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_GEOIP
    "ngx_stream_geoip_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_SPLIT_CLIENTS
    "ngx_stream_split_clients_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_RETURN
    "ngx_stream_return_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_LEAST_CONN
    "ngx_stream_upstream_least_conn_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_RANDOM
    "ngx_stream_upstream_random_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_UPSTREAM_ZONE
    "ngx_stream_upstream_zone_module",
#endif

#ifdef CONFIG_LIBNGINX_STREAM_SSL_PREREAD
    "ngx_stream_ssl_preread_module",
#endif
#endif

#ifdef CONFIG_LIBNSSL
    "ngx_openssl_module",
#endif

#ifdef CONFIG_LIBPCRE
    "ngx_regex_module",
#endif

    NULL
};
