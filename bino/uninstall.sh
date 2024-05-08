#!/bin/bash


INSTALL_DIR="/var/DJUMBAI"
# Usuários a serem removidos
userq="djumbaiq"
users="djumbais"

# Remover os usuários
userdel -r "$userq"
userdel -r "$users"

echo "Utilizadores removidos com sucesso."

rm -r "$INSTALL_DIR"
rm /usr/local/bin/djumbai-inject
rm /usr/local/bin/djumbai-start

echo "Pasta DJUMBAI removida com sucesso."
