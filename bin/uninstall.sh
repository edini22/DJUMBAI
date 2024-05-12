#!/bin/bash


INSTALL_DIR="/var/DJUMBAI"
# Usuários a serem removidos
userq="djumbaiq"
users="djumbais"
userg="djumbaig"

# Remover os usuários
userdel -r "$userq"  > /dev/null 2>&1
userdel -r "$users"  > /dev/null 2>&1
userdel -r "$userg"  > /dev/null 2>&1

echo "Users removed successfully"

rm -r "$INSTALL_DIR" > /dev/null 2>&1
rm /usr/local/bin/djumbai-inject > /dev/null 2>&1
rm /usr/local/bin/djumbai-check > /dev/null 2>&1
rm /usr/local/bin/djumbai-groups > /dev/null 2>&1
rm /usr/local/bin/djumbai-start > /dev/null 2>&1 
rm /usr/local/bin/djumbai-stop > /dev/null 2>&1 

rm /tmp/clean_pipe0 > /dev/null 2>&1
rm /tmp/clean_pipe1 > /dev/null 2>&1
rm /tmp/spawn_pipe0 > /dev/null 2>&1
rm /tmp/spawn_pipe1 > /dev/null 2>&1

echo "DJUMBAI removed successfully."

echo "Uninstallation completed"