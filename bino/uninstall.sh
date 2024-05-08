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

echo "Pasta DJUMBAI removida com sucesso."
