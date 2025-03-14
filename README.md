# 1-Project description:
a security camera using an esp 32 with an ov 2640 camera that sends an image using mail when there is movement detected you can also watch a livestream locally + a back up server using an esp 8266 in the case of internet unavailability or a power outage .
# 2-Prerequisites 
i used the the vscode-esp-idf-extension you can find here https://github.com/espressif/vscode-esp-idf-extension , and arduino as an ESP-IDF component you can setup with the help of this link https://docs.espressif.com/projects/arduino-esp32/en/latest/esp-idf_component.html.
# 3-Parts used
- esp 32 cam (ov 2640) 
- PIR Motion sensor 
- light bulb 
- Relay Module . 
- esp 8266 NodeMCU 
- sd card module
# 4-Wiring 
## a - camera : 
![image](https://github.com/user-attachments/assets/3d49b327-5ce9-4199-ad10-69667858e061)

## b - backup server :
![image](https://github.com/user-attachments/assets/e728c320-61ab-49d4-89a3-e49c5b12b845)

note: if you are experiencing issues with sd card initializations try using the 5v input instead of 3.3v .

# 5-code :
## a - camera :
check files>files_list in the [Doxygen Documentation](https://yassine-hattay.github.io/esp_32_sec_cam_1/index.html) for files description .

## b - backup server :
[Doxygen Documentation](https://yassine-hattay.github.io/esp_8266_bs/index.html)

# 6-how to use:

## a - camera :
this project is still in works , but if you wish to implement it as is right now here is how you would do it .

first booting up the esp 32 you will be greeted with this page (on connect page):
![image](https://github.com/user-attachments/assets/96ad777d-29c8-41ab-9da4-e5eac96e77ef)

clicking the live feed button does what you think it does:
![image](https://github.com/user-attachments/assets/23d8cca3-595c-49a1-8b9e-a1991c3d621a)

and clicking automated mode will give you this page :
![image](https://github.com/user-attachments/assets/b73f1512-6d07-4176-8c55-da9cfd6565d6)

in this mode if there is movment detected and an internet connexion the camera will take a photo and send it through mail , if there isn't internet connexion (and in future implementation even if there is a power outage) it will be saved to the backup server .

## b - backup server :

starting the backup server you will be greeted with this page :
![image](https://github.com/user-attachments/assets/c64c78ac-6857-4f24-9174-22f5493e6886)
clicking one of the dates will give you the pictures for that day and their time above each :
![image](https://github.com/user-attachments/assets/dd34e59c-6121-4107-940a-05866b29c022)

for the backup server to be able to recive files you must click end server button in the dates page .

note : i put a delay on most of the buttons so you will have to wait 5 sec to be able to click them this was done so the esp8266 doesn't crash and reboot , also only use the web pages buttons don't use the browser's back button or you will break the server .

 








    





    
