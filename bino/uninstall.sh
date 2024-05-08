#!/bin/bash

# Usuários a serem removidos
user1="djumbaiq"
user2="djumbais"

# Remover os usuários
sudo userdel -r "$user1"
sudo userdel -r "$user2"

echo "Usuários removidos com sucesso."
