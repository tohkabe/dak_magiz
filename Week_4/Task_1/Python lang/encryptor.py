import base64
import hashlib
from Crypto.Cipher import AES
from Crypto import Random

class AESCipher:
    def __init__(self, key: bytes):
        self.bs = AES.block_size
        self.key = hashlib.sha256(key).digest()

    def pad(self, raw: bytes) -> bytes:
        padding = self.bs - len(raw) % self.bs
        return raw + bytes([padding] * padding)

    def unpad(self, raw: bytes) -> bytes:
        return raw[:-raw[-1]]

    def encrypt(self, raw: bytes) -> bytes:
        raw = self.pad(raw)
        iv = Random.new().read(self.bs)
        cipher = AES.new(self.key, AES.MODE_CBC, iv)
        return iv + cipher.encrypt(raw)

    def decrypt(self, enc: bytes) -> bytes:
        iv = enc[:self.bs]
        cipher = AES.new(self.key, AES.MODE_CBC, iv)
        return self.unpad(cipher.decrypt(enc[self.bs:]))
