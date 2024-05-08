#!/bin/bash


INSTALL_DIR="/var/DJUMBAI"
# Usuários a serem removidos
userq="djumbaiq"
users="djumbais"
group1="xereca-roxa"
group2="buceta"

# Remover os usuários
sudo userdel -r "$userq"
sudo userdel -r "$users"
sudo groupdel -r "$group1"
sudo groupdel -r "$group2"

echo "Utilizadores removidos com sucesso."

rm -r "$INSTALL_DIR"

echo "Pasta DJUMBAI removida com sucesso."
