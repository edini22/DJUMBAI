#!/bin/bash


INSTALL_DIR="/var/DJUMBAI"
# Usuários a serem removidos
userq="djumbaiq"
users="djumbais"
group1="xereca-roxa"
group2="buceta"

# Remover os usuários
userdel -r "$userq"
userdel -r "$users"
groupdel "$group1"
groupdel "$group2"

echo "Utilizadores removidos com sucesso."

rm -r "$INSTALL_DIR"

echo "Pasta DJUMBAI removida com sucesso."
