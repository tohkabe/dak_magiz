import os
from encryptor import AESCipher

HARDCODED_KEY = b'my_super_secret_key!'

def decrypt_secret_folder():
    home = os.path.expanduser("~")
    target_folder = os.path.join(home, "secret_folder")

    if not os.path.exists(target_folder):
        print(f"[!] Target folder not found: {target_folder}")
        return

    cipher = AESCipher(HARDCODED_KEY)

    for root, dirs, files in os.walk(target_folder):
        for file in files:
            if not file.endswith(".locked"):
                continue

            locked_path = os.path.join(root, file)
            original_path = locked_path.replace(".locked", "")

            try:
                with open(locked_path, "rb") as f:
                    encrypted_content = f.read()

                decrypted_content = cipher.decrypt(encrypted_content)

                with open(original_path, "wb") as f:
                    f.write(decrypted_content)

                os.remove(locked_path)

                print(f"[+] Decrypted: {original_path}")
            except Exception as e:
                print(f"[-] Failed to decrypt {locked_path}: {e}")

if __name__ == "__main__":
    decrypt_secret_folder()
    print("\n[*] Decryption completed.\n")
