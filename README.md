# DJUMBAI

O DJUMBAI, do ponto de vista funcional, é um serviço que suporta o envio de mensagens para um utilizador unix local e a sua leitura pelo respetivo destinatário, de froma segura, permitindo ainda gerir utilizadores do serviço. Também, tem a noção de grupos privados de conversação, oferecendo mecanismos para criação de grupos, remoção de grupos e de gestão dos seus membros.

## Instalação

1. **cd bin && sudo make** (compila todo o código fonte)
2. **sudo bash install.sh** (cria toda a estrutura de pastas na diretoria ``/var/DJUMBAI``, cria os utilizadores necessários, **e links na pasta ``/usr/local/bin`` para execução dos programas**)
 
## Desinstalação

1. **sudo bash bin/uninstall.sh** (apaga todos os ficherios e pastas da diretoria ``/var/DJUMBAI``, utilizadores)

## Makefile

- **make \<programa\>** (compila o programa específicado)
- **make clean** (remove executáveis e ficheiros de objetos, bem como mensagens na queue e nas mailboxes, e outro tipo de ficheiros temporários)
- **make** (executa o **clean** e compila todos os programas)

### Links para Execução dos Programas

Para facilitar a execução dos comandos do DJUMBAI a partir de qualquer localização no sistema, são gerados links simbólicos na pasta `/usr/local/bin`. Isso permite que os seguintes comandos sejam executados em qualquer parte do sistema:

- **djumbai-start**
- **djumbai-stop**
- **djumbai-inject**
- **djumbai-check**
- **djumbai-groups**

Esses links são criados para os respectivos executáveis localizados em `/var/DJUMBAI/`, garantindo que os comandos possam ser facilmente acessados e utilizados em qualquer lugar do sistema sem a necessidade de especificar o caminho completo até o executável.


## Relatório

O relatório do serviço encontra-se na diretoria ``doc``



## Informações Relevantes sobre como enviar uma Mensagem

**Descrição:**
`djumbai-inject` é uma ferramenta utilizada para enviar mensagens para outros utilizadores ou grupos. Ele permite que o utilizador especifique o destinatário da mensagem, o assunto e o corpo da mensagem.

**Modo de Uso:**
1. Execute o comando `djumbai-inject` no terminal.
2. Será solicitado o UID (User ID) do destinatário da mensagem ou o nome de um grupo, caso seja utilizado com a opção `-g`.
3. Insira o assunto da mensagem, limitado a 200 caracteres, quando solicitado e pressione Enter.
4. Em seguida, você poderá redigir a mensagem, com um limite de 512 caracteres. Pressione Enter para começar uma nova linha ou **pressione Ctrl+D uma ou duas vezes para indicar o término da entrada de mensagem e enviá-la**.

## Informações Relevantes sobre como ler uma Mensagem

**Descrição:**
`djumbai-check` é uma ferramenta utilizada para verificar mensagens recebidas. Ele exibe as mensagens para o utilizador, destacando as mensagens não lidas em negrito e as mensagens lidas em texto normal. Permite também a leitura de mensagens específicas.

**Modo de Uso:**
1. Execute o comando `djumbai-check` no terminal.
2. Serão exibidas as mensagens recebidas, com as não lidas em negrito e as lidas em texto normal.
3. Para abrir e ler uma mensagem específica, utilize o comando `djumbai-check -g <numero_mensagem>`. O `<numero_mensagem>` corresponde ao número da mensagem na lista exibida, indicado na coluna mais à esquerda.

## Informações Relevantes sobre como gerir Grupos

**Descrição:**
O `djumbai-groups` é uma ferramenta utilizada para gerir grupos de utilizadores. Ele permite que o utilizador crie, remova e liste grupos, bem como adicione ou remova utilizadores de grupos existentes criados por si.

**Modo de Uso:**
- Para criar um novo grupo:
    ```
    djumbai-groups -c <nome> <utilizador1> ... <utilizadorN>
    ```
    O `<nome>` do grupo é limitado a um número específico de caracteres(20).

- Para adicionar um utilizador a um grupo existente:
    ```
    djumbai-groups -a <nome_grupo> <utilizador>
    ```

- Para remover um utilizador de um grupo:
    ```
    djumbai-groups -ru <nome_grupo> <utilizador>
    ```

- Para eliminar um grupo:
    ```
    djumbai-groups -rg <nome_grupo>
    ```

- Para listar todos os grupos a que pertence:
    ```
    djumbai-groups -lg
    ```

- Para listar os membros de um grupo específico:
    ```
    djumbai-groups -l <nome_grupo>
    ```

## Caso de uso

**`NOTA:` Sempre que é efetuado um `djumbai-check` pode haver a necessidade de esperar 60 segundos.*

#### 1. Arrancar o serviço

- **sudo djumbai-start**

#### 2. Verificar que não possui mensagens na mailbox
- **djumbai-check**


#### 3. Enviar uma mensagem

- **djumbai-inject**    

Inserir os seguintes dados:
```
Reciever uid: "1000"
Subject: "Teste exemplo"
Message: "Isto é uma mensagem de teste", Ctrl+D
```
#### 4. Verificar as mensagens presentes na mailbox
- **djumbai-check**

#### 5. Ler uma mensagem presente na mailbox
- **djumbai-check -g 0**

#### 6. Verificar que mensagem ficou marcada como lida
- **djumbai-check**

#### 7. Criar um grupo

- djumbai-groups -c `GRUPO_TESTE` `0` `1`

#### 8. Adicionar user ao grupo

- djumbai-groups -a `GRUPO_TESTE` `2`

#### 9. Enviar mensagem para um grupo
- djumbai-inject -g

Inserir os seguintes dados:
```
Reciever uid: "GRUPO_TESTE"
Subject: "Teste exemplo nº1 do grupo"
Message: "Isto é uma mensagem de teste para o GRUPO_TESTE", Ctrl+D
```

#### 10. Verificar se um elemento do grupo recebeu a mensagem
Tem de ser executado por cada utilizador do grupo, neste caso, o utilizador `0`, para isso a utilização do sudo:
- sudo djumbai-check 

#### 11. Ler a mensagem enviada para o grupo
- sudo djumbai-check -g 0

#### 12. Remover um utilizador do grupo
- djumbai-groups -ru `GRUPO_TESTE` `0`

#### 13. Verificar que o utilizador foi removido do grupo
- djumbai-groups -l `GRUPO_TESTE`

#### 14. Enviar mensagem para um grupo
- djumbai-inject -g

Inserir os seguintes dados:
```
Reciever uid: "GRUPO_TESTE"
Subject: "Teste exemplo nº2 do grupo"
Message: "Isto é uma mensagem de teste para o GRUPO_TESTE", Ctrl+D
```

#### 15. Verificar se o ex-elemento do grupo recebeu a mensagem
- **sudo djumbai-check**

#### 16. verificar a quais grupos pertenço
- djumbai-groups -lg

#### 17. Remover um grupo
- djumbai-groups -rg `GRUPO_TESTE`

#### 18. Verificar quais os utilizadores presentes no grupo
- djumbai-groups -l `GRUPO_TESTE`