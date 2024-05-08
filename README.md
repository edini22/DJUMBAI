# DJUMBAI
projeto TS3

- comunicação assíncrona e síncrona
- Tamanho das mensagens nao ultrapassar 512 bytes
- criação e exclusão de utilizadores
- criação e exclusão de grupos de utilizadores

# Estrutura 
- alias
- bin
- boot
- control
- doc
- man
- queue
    - intd
    - local
    - mess
    - pid
    - todo
- users

# Send
- todo/212321312.lnk
    - intd
        - from Rois to FIGOS,MELANCIAS

    - create info/212321312.mdjumbai
    - create local/212321312.mdjumbai
        - FIGOS [Not Done] 
        - MELANCIAS [Not Done]

    - clean[intd/todo/212321312] <- tag [removed]
    - if [removed]
        - modify local/212321312.mdjumbai
            - FIGOS [Not Done] -> lspawn[FIGOS] <- FIGOS [Done]
            - MELANCIAS [Not Done] -> lspawn[MELANCIAS] <- MELANCIAS [Done]

        - clean[mess/212321312] <- tag [removed]
        - if [removed]
            - remove local/212321312.mdjumbai and info/212321312.mdjumbai


- todo/212321313.lnk
- todo/212321314.lnk

# Permissoes

qmailq (queue, clean)
    * -w-
        - mess
        - intd
        - pid
        - todo
    
qmails (send)
    * rw-
        - info (envelope(sender and subject))
        - local (received with tag([Done]/[Not Done]))
    * r--
        - mess
        - intd
        - todo

# TODO

* handle SIGINT no djumbai-start para matar os filhos e fechar os pipes

* ERRO de criar uma pasta com o uid de outra pessoa

* resolver TODO'S

* criar grupos de utilizadores

* interface para os users verem as mensagens e ao ler a mensagem marcar como lida ( mudar de new para cur)

* comunicação síncrona

* Proteger input do Inject