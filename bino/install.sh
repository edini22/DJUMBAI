#!/bin/bash

# Função para obter o UID de um utilizador
get_user_uid() {
    id -u "$1"
}

# Criar utilizadores
user1="djumbaiq"
user2="djumbais"

sudo useradd "$user1"
sudo useradd "$user2"

# Obter UIDs dos utilizadores
uid_user1=$(get_user_uid "$user1")
uid_user2=$(get_user_uid "$user2")

# Escrever UIDs num arquivo
echo -e "$uid_user1" > uids.txt
echo -e "$uid_user2" >> uids.txt

echo "Uids dos utilizadores foram escritos no arquivo uids.txt."
