#!/bin/bash


INSTALL_DIR="/var/DJUMBAI"
# Usuários a serem removidos
userq="djumbaiq"
users="djumbais"
users="djumbaig"

# Remover os usuários
userdel -r "$userq"
userdel -r "$users"
userdel -r "$userg"

echo "Utilizadores removidos com sucesso."

rm -r "$INSTALL_DIR"
rm /usr/local/bin/djumbai-inject
rm /usr/local/bin/djumbai-check
rm /usr/local/bin/djumbai-groups
rm /usr/local/bin/djumbai-start

echo "Pasta DJUMBAI removida com sucesso."
