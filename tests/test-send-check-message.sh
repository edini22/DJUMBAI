#!/bin/bash

# Defina o diretório onde estão os arquivos
diretorio="/var/DJUMBAI/users/0/new/"

# Variável com o valor a ser comparado
valor="TO: [0]
FROM: 0
SUBJECT: Teste Lindo
MESSAGE: Mensagem WS

"

UID="0"
# Segunda string
subject="Teste Lindo"

# Terceira string terminada com Ctrl+D
message=$(cat <<EOF
Mensagem WS
EOF
)

# Execute o programa com as três strings
echo -e "$UID\n$subject\n$message" | djumbai-inject

echo "Message sent"

echo "Waiting 60 seconds..."
# sleep 60 sec
sleep 60

# Define as cores para as mensagens de sucesso e falha
cor_sucesso="\e[32m"  # Verde
cor_falha="\e[31m"    # Vermelho
cor_reset="\e[0m"     # Resetar a cor

# Variável para verificar se a mensagem foi encontrada
mensagem_encontrada=false


# Itere sobre os arquivos no diretório
for arquivo in "$diretorio"/*; do
    # Verifica se o arquivo é um arquivo regular
    if [ -f "$arquivo" ]; then
        # Verifica se a mensagem está presente no arquivo
        if grep -q "$valor" "$arquivo"; then
            # Se a mensagem for encontrada, imprime uma mensagem verde
            echo -e "${cor_sucesso}Message found in file: $arquivo${cor_reset}"
            mensagem_encontrada=true
            rm "$arquivo"
            break
        fi
    fi
done


# Verifica se a mensagem foi encontrada
if [ "$mensagem_encontrada" = true ]; then
    # Se a mensagem foi encontrada, termina com status 0
    exit 0
else
    # Se a mensagem não foi encontrada, imprime uma mensagem vermelha e termina com status 1
    echo -e "${cor_falha}No message was found${cor_reset}"
    exit 1
fi
