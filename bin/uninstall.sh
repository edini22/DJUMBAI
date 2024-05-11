#!/bin/bash


INSTALL_DIR="/var/DJUMBAI"
# Usuários a serem removidos
userq="djumbaiq"
users="djumbais"
userg="djumbaig"

# Remover os usuários
userdel -r "$userq"
userdel -r "$users"
userdel -r "$userg"

echo "Users removed successfully"

rm -r "$INSTALL_DIR"
rm /usr/local/bin/djumbai-inject
rm /usr/local/bin/djumbai-check
rm /usr/local/bin/djumbai-groups
rm /usr/local/bin/djumbai-start 
rm /usr/local/bin/djumbai-stop 

echo "DJUMBAI removed successfully."

echo "Uninstallation completed"