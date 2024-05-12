#!/bin/bash

djumbai-groups -c TESTE 1

echo "Order for group creation sent"

echo "Waiting 5 seconds..."

# sleep 60 sec
sleep 5

# Define as cores para as mensagens de sucesso e falha
cor_sucesso="\e[32m"  # Verde
cor_falha="\e[31m"    # Vermelho
cor_reset="\e[0m"     # Resetar a cor

# Variável para verificar se a mensagem foi encontrada
mensagem_encontrada=false


# Defina o nome do arquivo e o conteúdo esperado
nome_arquivo="/var/DJUMBAI/groups/TESTE.mdjumbai"
conteudo_esperado="0
2
1
"

# Verifique se o arquivo existe
if [ -f "$nome_arquivo" ]; then
    # Verifique se o conteúdo do arquivo é o esperado
    if grep -q "$conteudo_esperado" "$nome_arquivo"; then
        echo -e "${cor_sucesso}Group creation sucessfull${cor_reset}"
            mensagem_encontrada=true
        exit 0  # Saia com status de sucesso
    else
        echo -e "${cor_falha}Group creation failed${cor_reset}"
        exit 1  # Saia com status de falha
    fi
else
    echo -e "${cor_falha}Group creation failed${cor_reset}"
    exit 1  # Saia com status de falha
fi
