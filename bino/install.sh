#!/bin/bash

INSTALL_DIR="/var/DJUMBAI"
BIN_DIR="$INSTALL_DIR/bin"
BOOT_DIR="$INSTALL_DIR/boot"
QUEUE_DIR="$INSTALL_DIR/queue"
USERS_DIR="$INSTALL_DIR/users"
GROUP_DIR="$INSTALL_DIR/groups"

# Função para obter o UID de um utilizador
get_user_uid() {
    id -u "$1"
}

# Criar utilizadores
userq="djumbaiq"
users="djumbais"
userg="djumbaig"

useradd "$userq"
useradd "$users"
useradd "$userg"

# Obter UIDs dos utilizadores
uid_userq=$(get_user_uid "$userq")
uid_users=$(get_user_uid "$users")
uid_userg=$(get_user_uid "$userg")


# Criar diretório de instalação
echo "A criar diretorias bin ..."
mkdir -p "$BIN_DIR"

cp ./djumbai-inject ./djumbai-queue ./djumbai-send ./djumbai-clean ./djumbai-lspawn ./djumbai-local ./djumbai-check ./djumbai-groups ./djumbai-group-manager "$BIN_DIR"

chmod 701 "$BIN_DIR/djumbai-inject"

chown "$userq" "$BIN_DIR/djumbai-queue"
chmod 701 "$BIN_DIR/djumbai-queue"
chmod u+s "$BIN_DIR/djumbai-queue"

chown "$users" "$BIN_DIR/djumbai-send"
chmod 700 "$BIN_DIR/djumbai-send"

chown "$userq" "$BIN_DIR/djumbai-clean"
chmod 700 "$BIN_DIR/djumbai-clean"

chmod 700 "$BIN_DIR/djumbai-lspawn"

chmod 711 "$BIN_DIR/djumbai-local"

chmod 711 "$BIN_DIR/djumbai-check"

chmod 711 "$BIN_DIR/djumbai-groups"

chown "$userg" "$BIN_DIR/djumbai-group-manager"
chmod 701 "$BIN_DIR/djumbai-group-manager"
chmod u+s "$BIN_DIR/djumbai-group-manager"

echo "Diretorias bin criadas!"

# Criar diretório boot
echo "A cria diretoria boot ..."

mkdir -p "$BOOT_DIR"

cp ../boot/djumbai-start "$BOOT_DIR"
chmod 700 "$BOOT_DIR/djumbai-start"

echo "Diretoria boot criada!"

# Criar diretório queue

echo "A criar diretorias queue ..."

mkdir -p "$QUEUE_DIR"
chmod 755 "$QUEUE_DIR"

mkdir -p "$QUEUE_DIR/info"
chown "$users" "$QUEUE_DIR/info"
chmod 700 "$QUEUE_DIR/info"

mkdir -p "$QUEUE_DIR/local"
chown "$users" "$QUEUE_DIR/local"
chmod 700 "$QUEUE_DIR/local"

mkdir -p "$QUEUE_DIR/intd"
chown "$userq":"$users" "$QUEUE_DIR/intd"
chmod 2350 "$QUEUE_DIR/intd"

mkdir -p "$QUEUE_DIR/pid"
chown "$userq" "$QUEUE_DIR/pid"
chmod 300 "$QUEUE_DIR/pid"

mkdir -p "$QUEUE_DIR/mess"
chown "$userq":"$users" "$QUEUE_DIR/mess"
chmod 2350 "$QUEUE_DIR/mess"

mkdir -p "$QUEUE_DIR/todo"
chown "$userq":"$users" "$QUEUE_DIR/todo"
chmod 2350 "$QUEUE_DIR/todo"

echo "Diretorias queue criadas!"

# Criar diretório users

echo "A criar diretoria users ..."

mkdir -p "$USERS_DIR"
chmod 755 "$USERS_DIR"

echo "A criar diretoria groups ..."

mkdir -p "$GROUP_DIR"
chown "$userg":"$users" "$GROUP_DIR"
chmod 2750 "$GROUP_DIR"

mkdir -p "$GROUP_DIR/users"
chown "$userg" "$GROUP_DIR/users"
chmod 700 "$GROUP_DIR/users"

echo "Diretoria groups criadas!"


# Escrever UIDs num arquivo
echo -e "$uid_userq" > "$BIN_DIR/uids.txt"
echo -e "$uid_users" >> "$BIN_DIR/uids.txt"
echo -e "$uid_userg" >> "$BIN_DIR/uids.txt"

echo "Uids dos utilizadores foram escritos no arquivo uids.txt."


echo "Install concluído!"

ln -s /var/DJUMBAI/bin/djumbai-inject /usr/local/bin/djumbai-inject
ln -s /var/DJUMBAI/bin/djumbai-check /usr/local/bin/djumbai-check
ln -s /var/DJUMBAI/bin/djumbai-groups /usr/local/bin/djumbai-groups
ln -s /var/DJUMBAI/boot/djumbai-start /usr/local/bin/djumbai-start
