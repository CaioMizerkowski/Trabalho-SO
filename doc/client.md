## Cliente

Usando a mesma porta para conexão que o servidor (padrão 5193) e possuindo duas threads necessárias para fazer a sincronização entre o envio de dados para o servidor e a recepção dos dados do servidor, ele consiste em três funções principais: recebeDados, enviaDados e main.

A main é responsável por iniciar o socket compartilhado pelas threads, conectar ele ao host no ip e porta corretos, iniciar o semáforo mutex corretamente, iniciar as threads e fazer o join entre elas, de forma que elas não virem processos zumbis. Com a finalização de ambas as threads, ele também é responsável por fechar o socket. Para isso, ela usa as bibliotecas socket, pthread e semaphore.

A thread recebeDados é responsável por fazer a leitura do socket e mostrar estes dados na tela. É usada a função recv para fazer a leitura do socket, e a função printf para mostrar os dados na tela. Caso a função recebe a mensagem 'Adeus', significa que o cliente foi desconectado, então a thread é finalizada. Para funcionar de maneira síncrona com a thread enviaDados, é usado o mutex, fazendo-se o sem_post nesta função sempre que algo é recebido do servidor.

A thread enviaDados é responsável por escrever no socket e enviar os dados para o servidor. É usada a função send para enviar os dados para o servidor. Para funcionar de maneira síncrona com a thread recebeDados, é usado o mutex, fazendo-se o sem_wait nesta função sempre que algo é enviado para o servidor. Quando a função envia 'FIM' para o servidor, significa que o cliente deseja encerrar a conexão, então a thread é finalizada.

Com a finalização de ambas as threads, a main retorna ao seu ciclo após o fim dos pthread_join e fecha o socket.
