# Trabalho de Sistemas Operacionais

O trabalho a seguir foi realizado conforme a supervisão do prof. Pedroso na matéria TE355, Sistemas Operacionais Embarcados, no qual o objetivo é aprofundar o entendimento das necessidades do uso de semáforos em sistemas multithread. Para tal foi entregue um servidor em estado parcialmente funcional para ter sua implementação finalizada com a adição de funções para o protocolo de comunicação estabelecido entre o servidor e o cliente. O protocolo visa controlar as alterações, escritas e gravações em um array de ditados e permitir a leitura dos ditados pelo cliente.

O cliente também teve que ser finalizado, com a implementação de funções especificas de forma a automatizar o envio de requisições ao servidor para permitir a consulta por múltiplos clientes, forçando a necessidade do uso de semáforos. Sendo a comunicação entre o servidor e o cliente feita através de sockets deixamos previamente configurados no código usado como base.

## Introdução

Devido as dificuldades de se compartilhar recursos entre processos, a estratégia da utilização de threads foi implementada tanto no ambiente windows como nos ambientes unix-like (Linux, Mac, etc). O uso de threads permite com que recursos definidos como globais no escopo do programa sejam compartilhados, permitindo com que várias tarefas acessem os mesmos recursos. No entanto, essa estratégia cria a necessidade de um processo de sincronização entre as threads de forma a que duas ou mais tarefas não utilizem o mesmo recurso simultaneamente.

Para resolver esse problema foi implementado o uso de semáforos, que são estruturas de controle que permitem que duas ou mais threads possam acessar recursos compartilhados de forma a evitar que este recurso seja utilizado por duas ou mais threads simultaneamente. Os semáforos, ao contrário de outras tentativas de sincronizar as threads como por meio de flags e de esperas ocupadas, se utilizam de recursos disponibilizados pelo hardware e pelo sistema operacional para bloquearem a execução de uma thread, que solicita um recurso ocupado, até que o recurso seja liberado pela thread que o solicitou.

Isso é extremamente relevante em sistemas do tipo cliente-servidor, no qual um servidor deve atender diversos clientes e cuja uma das estratégias para seu funcionamento é subir uma thread para cada cliente, de forma que os recursos utilizados por todas estas threads devem ser protegidos de forma a evitar que estes sejam utilizados por duas ou mais threads simultaneamente. Um exemplo prático disso é a proteção do array de ditados do problema proposto, que deve ser protegido de escritas e deleções simultâneas de forma a não existirem furos em seu conteúdo, como espaços vazios que não deveriam estar, duplicação de informações ou uma diferença entre o número de ditados realmente presente no arquivo e o número presente na variável que controla o número de ditados a serem gravados ao final do processo.

A comunicação entre o cliente e o servidor também trás outras dificuldades de sincronização que também necessitam do uso de semáforos para a correção através da proteção de regiões críticas do código. Neste caso, é a proteção do identificador do socket que deve ser passado para dentro da thread correspondente ao atendimento daquela conexão. Como o valor da thread é passado como um ponteiro para o valor, que então deve ser enviado para dentro da thread e salvo em uma variável do tipo int de escopo local. Durante todo este processo, existe a possibilidade da execução da thread ser preempetadas e outra thread assumir o local, modificando indevidamente o valor do socket e causando uma falha de comunicação entre o servidor e o cliente. Sendo que enquanto um cliente fica esperando por respostas e envios de uma thread que não existe, outro cliente passa a enviar para duas ou mais threads sem nenhuma garantia sobre qual thread vai atender os seus envios e em qual momento.

## Servidor

O código do servidor recebido foi expandido de forma a adicionar uma serie de funções, com algumas modificações na main tendo sido realizadas e diversas modificações na função responsável por atender a conexão de cada cliente, através da criação de uma thread que inicia esta função.

### Main

Dois semáforos são iniciados na main, chamados de *m* e *m2*, os dois iniciados com o valor de 1 e devendo ser usados somente para a criação de regiões de exclusão mutua, sendo estes semáforos também são chamados de mutex. A principal modificação na main se deu através do uso do segundo destes dois semáforos (*m2*) para proteger a região crítica dentro do loop que recebe as requisições dos clientes para se conectarem com o servidor através do socket disponibilizado. Neste trecho de código, o semáforo somente é submetido a operação **Down**, pois o fim de sua região crítica só ocorre dentro da thread após o valor do socket ter sido finalmente fixado na variável de escopo local. No código do servidor apresentado em anexo, essa região crítica se inicia na linha 455 dentro da função main e termina na linha 125, dentro da função atenderConexão, com a operação **Up** sendo realizada no semáforo.

Em pseudo código, podemos representar essa proteção de exclusão mútua da seguinte maneira:
PSEUDOCÓDIGO AQUI

### Atende Conexão

Na função atendeConexão, que deve existir idealmente numa thread separada para cada cliente que esteja se comunicando com o servidor, foram conferidas as regiões de exclusão mútua das funcionalidades já existentes e adicionadas novas funcionalidades, sempre observando quais são as regiões de exclusão mútua presentes nas mesmas.

Todas as funcionalidades esperadas foram implementadas com sucesso, adicionalmente uma funcionalidade de *HELP* foi também inserida para facilitar o debug na comunicação interativa com o servidor através do cliente. As funcionalidades e suas regiões de exclusão mutuas são as seguintes:

--Adicionar tabela de funcionalidades e regiões, com observações.

Dando-se enfase as funções de auxilio criadas para as funcionalidades mais complexas.

A função *remove_element* lida com a remoção do elemento e a eliminação de elementos vazios no meio do array de ditados, movendo todos os ditados com um índice maior do que o ditado excluído uma posição para trás e decrementando o contador de ditados. Toda essa operação ocorrendo dentro de uma região crítica.

A função de *busca* itera sobre todos os ditados, buscando aquele que possui como uma substring o termo buscado. Também estando inteiramente dentro da região crítica, pois itera sobre o array de ditados (msg).

A função *palavras-d* é usada tanto na contagem de palavras individuais dos ditados, como na contagem de palavras de todos os ditados, em ambos os casos dentro das regiões demarcadas para exclusão mútua. A diferença entre ambos os casos é que no segundo uma variável acumula os resultados retornados pela função, que é iterada sobre todos os ditados em um laço for. Enquanto que no primeiro caso não existe está iteração e somente um índice é consultado.

A função GravaDitado é uma adaptação da função LeDitado, disponibilizada pelo professor, no qual ao invés de se fazer a leitura, é feita a escrita em um arquivo especificado. A função LeDitado foi alterada para receber como parâmetro o nome do arquivo que possui os ditados, funcionalidade também presente na função GravaDitado, que recebe como parâmetro o nome do arquivo.

Para avisar ao cliente que a função foi corretamente executada, o servidor enviar um sinal de *ACK* ao final de cada execução com sucesso.

## Cliente

O cliente foi implementado através da expansão do código recebido como base, com a adição de duas threads, uma para receber os dados do servidor e outra para enviar os dados ao servidor, estando elas sincronizadas através do uso de um semáforo e com a opção de executar elas de modo interativa e não iterativo, enviando finitas vezes um comando para o servidor dentro de um loop sincronizado com a recepção das respostas.

### Main m

A função main do servidor é responsável pela conexão do programa através do socket, cujo código para o funcionamento foi disponibilizado no modelo. O semáforo para sincronização também é iniciado na mesma e um argumento extra foi adicionado a chamada da main. Este argumento é responsável por definir o modo de operação da cliente, se será interativo (0 ou vazio), executará deleções (1), rotações (2), substituições (3), leituras aleatórias (4) ou gravará os resultados (5).

### Envia Dados

A função enviaDados é responsável pelo envio de mensagens ao servidor, estando a thread da mesma sincronizada com a thread da função responsável por receber a resposta do servidor, essa sincronização ocorrendo por meio de um semáforo compartilhado. Esse semáforo só é usado nos casos de automação e seu funcionamento é ativado por meio de uma variável lock de escopo global, estando desabilitado o semáforo no modo interativo.

Ela fica desativada no modo interativo, pois a velocidade de envio dos comandos é menor e a sequência dos mesmos não determinística. Enquanto que nos casos automatizados, a velocidade de envio de comandos é muito rápida e a sequência totalmente determinada previamente.

Nos casos não interativos, o travamento ocorre pela operação **Down** aplicada ao semáforo compartilhado entre as threads, esta operação acontece a cada iteração do loop. Ficando a thread responsável por enviar os dados em espera até que servidor retorne um sinal de *ACK*.

No primeiro caso de execução automatizada de comandos, o caso da deleção, esta operação é repetida 100 vezes nas últimas posições do array de ditados.

No caso seguinte, o de rotação, esta operação é realizada 500 vezes, incrementando sempre o índice do ditado que será afetado.

No caso da execução das operações de *replace*, esse comando também está setado para ser executado 500 vezes, alterando o ditado afetado e jogando um texto informativo no lugar.

No quarto caso, leituras aleatórias são executadas 500 vezes pela thread através do comando *GETR*, usado principalmente para ocupar o servidor, já que está operação não faz alterações nos ditados.

A última função, é responsável por salvar os ditados daquela thread utilizando o número do socket como parte do nome do arquivo.

### Recebe Dados

Estando responsável por receber os dados do servidor ao cliente, a função recebeDados é sincronizada a thread de enviar dados, nos casos automatizados, através de um semáforo. Sendo nestes casos ela a responsável por executar a operação **Up** no semáforo de tamanho um.

Para acomodar os casos interativos e não interativos, um lock global foi aplicado. Sendo o mesmo responsável por definir se a operação **Up** será ou não executada a cada vez que o cliente receber o sinal de *ACK* do servidor.

## Metodologia

## Resultados

## Lixo

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
