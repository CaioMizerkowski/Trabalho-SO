## Servidor

O servidor, dentro de uma estrutura clientes/servidor, é responsável por receber e enviar mensagens aos clientes, gerenciando as diversas conexões. No caso do servidor presente neste trabalho, é utilizada a biblioteca de socket para fazer a conexão com os clients, e a biblioteca de pthread para criar as threads individuais para cada cada client. Sendo feito a proteção das regiões críticas através do uso de semáforos, do tipo mutex.

Um semáforo teve que ser usado de forma a prevenir que os clientes compartilhassem os sockets, pela possibilidade de preempção do processo durante a abertura da thread e da definição do socket presente dentro da mesma.

Enquanto que outro foi utilizado para proteger as regiões críticas dentro das threads para os momentos de comunicação com os clients. Tendo sido as identificadas as regiões críticas como as regiões que acessavam as variáveis globais compostas pelo dicionário de ditados, pela contagem de alterações, pela contagem de visitas e pelo número de ditados existentes, e que realizam a escrita, modificação ou leitura destas variáveis globais.

A função atendeConexao, executando em threads separadas por clients, é responsável por receber as requisições e enviar as respostas. Cada client tendo para si uma thread exclusiva e devendo as operações críticas dentro da thread serem realizadas de forma a proteger impedir o acesso mútuo aos recursos compartilhados entre as threads (variáveis globais).

O servidor após feita a conexão e iniciada a thread, em um loop infinito, recebe requisições do client e as identificas de forma a efetuar as operações solicitadas.

(descrição das operações na descrição do trabalho, pegar de lá)

GETR
GETN
REPLACE
VER
DEL
ROTATE
SEARCH
PALAVRAS-D
PALAVRAS-T
ALTERACOES
GRAVA
LE

A função atendeConexao também é responsável por verificar se o cliente enviou uma mensagem de saída 'FIM', caso isso aconteça, a função atendeConexao sai do loop e encerra a conexão com o client fechando o socket que foi aberto e enviando o sinal de saída para o client.

No envio e recebimento de informações dos clients, o servidor utiliza as funções recv e send do socket para receber e enviar as informações. 