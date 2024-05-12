#!/bin/bash

INSTALL_DIR="/var/DJUMBAI"
BIN_DIR="$INSTALL_DIR/bin"
BOOT_DIR="$INSTALL_DIR/boot"
QUEUE_DIR="$INSTALL_DIR/queue"
USERS_DIR="$INSTALL_DIR/users"
GROUP_DIR="$INSTALL_DIR/groups"
LOGS_DIR="$INSTALL_DIR/log"

# Função para obter o UID de um utilizador
get_user_uid() {
    id -u "$1"
}

# Criar utilizadores
userq="djumbaiq"
users="djumbais"
userg="djumbaig"

useradd "$userq" > /dev/null 2>&1
useradd "$users" > /dev/null 2>&1
useradd "$userg" > /dev/null 2>&1

# Obter UIDs dos utilizadores
uid_userq=$(get_user_uid "$userq")
uid_users=$(get_user_uid "$users")
uid_userg=$(get_user_uid "$userg")


# Criar diretório de instalação

mkdir -p "$BIN_DIR"

cp ./djumbai-inject ./djumbai-queue ./djumbai-send ./djumbai-clean ./djumbai-lspawn ./djumbai-local ./djumbai-check ./djumbai-groups ./djumbai-group-manager "$BIN_DIR"

chmod 101 "$BIN_DIR/djumbai-inject"

chown "$userq":"$userq" "$BIN_DIR/djumbai-queue"
chmod 6111 "$BIN_DIR/djumbai-queue"
# chmod u+s "$BIN_DIR/djumbai-queue"

chown "$users":"$users" "$BIN_DIR/djumbai-send"
chmod 110 "$BIN_DIR/djumbai-send"

chown "$userq":"$userq" "$BIN_DIR/djumbai-clean"
chmod 110 "$BIN_DIR/djumbai-clean"

chmod 100 "$BIN_DIR/djumbai-lspawn"

chmod 111 "$BIN_DIR/djumbai-local"

chmod 111 "$BIN_DIR/djumbai-check"

chmod 111 "$BIN_DIR/djumbai-groups"

chown "$userg":"$userg" "$BIN_DIR/djumbai-group-manager"
chmod 4111 "$BIN_DIR/djumbai-group-manager"

echo "/bin directory created"

# Criar diretório boot

mkdir -p "$BOOT_DIR"
chmod 500 "$BOOT_DIR"

cp ./djumbai-start "$BOOT_DIR"
chmod 100 "$BOOT_DIR/djumbai-start"

cp ../boot/djumbai-stop "$BOOT_DIR"
chmod 100 "$BOOT_DIR/djumbai-stop"

echo "/boot directory created"

# Criar diretoria queue

mkdir -p "$QUEUE_DIR"
chmod 755 "$QUEUE_DIR"

mkdir -p "$QUEUE_DIR/info"
chown "$users":"$users" "$QUEUE_DIR/info"
chmod 700 "$QUEUE_DIR/info"

mkdir -p "$QUEUE_DIR/local"
chown "$users":"$users" "$QUEUE_DIR/local"
chmod 700 "$QUEUE_DIR/local"

mkdir -p "$QUEUE_DIR/intd"
chown "$userq":"$users" "$QUEUE_DIR/intd"
chmod 2350 "$QUEUE_DIR/intd"

mkdir -p "$QUEUE_DIR/pid"
chown "$userq":"$userq" "$QUEUE_DIR/pid"
chmod 700 "$QUEUE_DIR/pid"

mkdir -p "$QUEUE_DIR/mess"
chown "$userq":"$users" "$QUEUE_DIR/mess"
chmod 2350 "$QUEUE_DIR/mess"

mkdir -p "$QUEUE_DIR/todo"
chown "$userq":"$users" "$QUEUE_DIR/todo"
chmod 2350 "$QUEUE_DIR/todo"

echo "/queue directory created"

# Criar diretoria users

mkdir -p "$USERS_DIR"
chmod 755 "$USERS_DIR"

echo "/users directory created"

mkdir -p "$GROUP_DIR"
chown "$userg":"$userq" "$GROUP_DIR"
chmod 2750 "$GROUP_DIR"

mkdir -p "$GROUP_DIR/users"
chown "$userg" "$GROUP_DIR/users"
chmod 700 "$GROUP_DIR/users" 

echo "/groups directory created"

mkdir -p "$LOGS_DIR"
touch "$LOGS_DIR/djumbai-inject.log"
chmod 600 "$LOGS_DIR/djumbai-inject.log"

touch "$LOGS_DIR/djumbai-check.log"
chmod 600 "$LOGS_DIR/djumbai-check.log"

touch "$LOGS_DIR/djumbai-groups.log"
chmod 600 "$LOGS_DIR/djumbai-groups.log"

touch "$LOGS_DIR/djumbai-start-stop.log"
chmod 600 "$LOGS_DIR/djumbai-start-stop.log"

touch "$LOGS_DIR/djumbai-queue.log"
chown "$userq":"$userq" "$LOGS_DIR/djumbai-queue.log"
chmod 600 "$LOGS_DIR/djumbai-queue.log"

touch "$LOGS_DIR/djumbai-send.log"
chown "$users":"$users" "$LOGS_DIR/djumbai-send.log"
chmod 600 "$LOGS_DIR/djumbai-send.log"

touch "$LOGS_DIR/djumbai-clean.log"
chown "$userq":"$userq" "$LOGS_DIR/djumbai-clean.log"
chmod 600 "$LOGS_DIR/djumbai-clean.log"

touch "$LOGS_DIR/djumbai-lspawn.log"
chmod 600 "$LOGS_DIR/djumbai-lspawn.log"

touch "$LOGS_DIR/djumbai-local.log"
chmod 600 "$LOGS_DIR/djumbai-local.log"

touch "$LOGS_DIR/djumbai-group-manager.log"
chown "$userg":"$userg" "$LOGS_DIR/djumbai-group-manager.log"
chmod 600 "$LOGS_DIR/djumbai-group-manager.log"

echo "/logs directory created"


echo -e "$uid_userq" > "$BIN_DIR/uids.txt"
echo -e "$uid_users" >> "$BIN_DIR/uids.txt"
echo -e "$uid_userg" >> "$BIN_DIR/uids.txt"


ln -s /var/DJUMBAI/bin/djumbai-inject /usr/local/bin/djumbai-inject > /dev/null 2>&1
ln -s /var/DJUMBAI/bin/djumbai-check /usr/local/bin/djumbai-check > /dev/null 2>&1
ln -s /var/DJUMBAI/bin/djumbai-groups /usr/local/bin/djumbai-groups > /dev/null 2>&1
ln -s /var/DJUMBAI/boot/djumbai-start /usr/local/bin/djumbai-start > /dev/null 2>&1
ln -s /var/DJUMBAI/boot/djumbai-stop /usr/local/bin/djumbai-stop > /dev/null 2>&1

echo "Symbolic links created"

echo "Installation completed"