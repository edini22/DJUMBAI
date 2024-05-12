#!/bin/bash

djumbai-groups -rg TESTE

echo "Order for group creation sent"

echo "Waiting 5 seconds..."

sleep 5

cor_sucesso="\e[32m"
cor_falha="\e[31m"
cor_reset="\e[0m"

mensagem_encontrada=false


nome_arquivo="/var/DJUMBAI/groups/TESTE.mdjumbai"


if [ -f "$nome_arquivo" ]; then
    echo -e "${cor_falha}Group removal failed${cor_reset}"
    exit 1
else
    echo -e "${cor_sucesso}Group removal sucessfull${cor_reset}"
    exit 0
fi
