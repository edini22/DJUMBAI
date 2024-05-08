#!/bin/bash

INSTALL_DIR="/var/DJUMBAI"
BIN_DIR="$INSTALL_DIR/bin"
BOOT_DIR="$INSTALL_DIR/boot"
QUEUE_DIR="$INSTALL_DIR/queue"
USERS_DIR="$INSTALL_DIR/users"

# Função para obter o UID de um utilizador
get_user_uid() {
    id -u "$1"
}

# Criar utilizadores
userq="djumbaiq"
users="djumbais"
group1="xereca-roxa"
group2="buceta"

useradd "$userq"
useradd "$users"
groupadd "$group1"
usermod -aG "$group1" "$users"
groupadd "$group2"
usermod -aG "$group2" "$userq"
usermod -aG "$group2" "$users"

# Obter UIDs dos utilizadores
uid_userq=$(get_user_uid "$userq")
uid_users=$(get_user_uid "$users")

# Escrever UIDs num arquivo
echo -e "$uid_userq" > uids.txt
echo -e "$uid_users" >> uids.txt

echo "Uids dos utilizadores foram escritos no arquivo uids.txt."

# Criar diretório de instalação
mkdir -p "$BIN_DIR"

cp ./djumbai-inject ./djumbai-queue ./djumbai-send ./djumbai-clean ./djumbai-lspawn ./djumbai-local "$BIN_DIR"

chmod 701 "$BIN_DIR/djumbai-inject"

chown "$userq" "$BIN_DIR/djumbai-queue"
chmod 701 "$BIN_DIR/djumbai-queue"

chown "$users" "$BIN_DIR/djumbai-send"
chmod 700 "$BIN_DIR/djumbai-send"

chown "$userq" "$BIN_DIR/djumbai-clean"
chmod 700 "$BIN_DIR/djumbai-clean"

chmod 700 "$BIN_DIR/djumbai-lspawn"

chmod 701 "$BIN_DIR/djumbai-local"

# Criar diretório boot

mkdir -p "$BOOT_DIR"

cp ../boot/djumbai-start "$BOOT_DIR"
chmod 700 "$BOOT_DIR/djumbai-start"

# Criar diretório queue

mkdir -p "$QUEUE_DIR"
chown "$userq":"$group1" "$QUEUE_DIR"
chmod 550 "$QUEUE_DIR"

mkdir -p "$QUEUE_DIR/info"
chown "$users" "$QUEUE_DIR/info"
chmod 700 "$QUEUE_DIR/info"

mkdir -p "$QUEUE_DIR/local"
chown "$users" "$QUEUE_DIR/local"
chmod 700 "$QUEUE_DIR/local"

mkdir -p "$QUEUE_DIR/intd"
chown "$userq":"$group1" "$QUEUE_DIR/intd"
chmod 350 "$QUEUE_DIR/intd"

mkdir -p "$QUEUE_DIR/pid"
chown "$userq":"$group1" "$QUEUE_DIR/pid"
chmod 350 "$QUEUE_DIR/pid"

mkdir -p "$QUEUE_DIR/mess"
chown "$userq":"$group1" "$QUEUE_DIR/mess"
chmod 350 "$QUEUE_DIR/mess"

mkdir -p "$QUEUE_DIR/todo"
chown "$userq":"$group1" "$QUEUE_DIR/todo"
chmod 350 "$QUEUE_DIR/todo"


# Criar diretório users

mkdir -p "$USERS_DIR"
chmod 707 "$USERS_DIR"
