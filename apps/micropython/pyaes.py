import random
import cryptolib
import float_operation
import helloworld


def generate(length):
    letters = "abcdefghijklmnopqrstuvwxyz1234567890"
    return ''.join(random.choice(letters) for i in range(length))


def main():
    latencies = {}
    timestamps = {}
    
    length_of_message = 1000
    num_of_iterations = 100
    message = generate(length_of_message)

    # 128-bit key (16 bytes)
    KEY = b'\xa1\xf6%\x8c\x87}_\xcd\x89dHE8\xbf\xc9,'

    for loops in range(num_of_iterations):
        aes = cryptolib.aes(KEY, 6, KEY)
        ciphertext = aes.encrypt(message)

        aes = cryptolib.aes(KEY, 6, KEY)
        plaintext = aes.decrypt(ciphertext)
        aes = None


    print ("Done")

if __name__ == "__main__":
    main()
