Ciobanu Alin-Matei 325CB

	Tema contine 3 surse .c si un header care le uneste.
	Ideea temei:
	- am un socket tcp pasiv pe care fac listen
	- un socket udp de pe care primesc datagrame
	- in while multiplexe intre mesajele venite de la tastatura, de la socketul
	tcp sau de la socketul udp
	- daca primesc pe socketul tcp inseamna ca este o cerere de conectare
	si fac accept
	- daca primesc de la udp, inseamna ca este un mesaj udp care trebie prelucrat
	si trimis destinatarului
	- daca primesc pe unul dintre clientii conectati inseamna ca vrea sa faca
	subscribe sau unsubscribe la un topic

	Pentru a tine evidenta topicurilor, a clientilor si a mesajelor stocate
	folosesc liste:
	- o lista de topicuri in care fiecare celula contine un camp topic si o lista
	de clienti, reprezentand clientii abonati la acel topic.
	- lista de clienti, fiecare client are campurile: id_client, optiunea
	 de SF, socketul si o lista store in care se stocheaza mesajele in cazul in
	care clientul este deconectat de la server, dar are optinea SF pe 1.
	
	Pentru a sti clientii conectati pe server, folosesc tot o lista
	de clienti cu aceleasi campuri. Fiindca o folosesc doar la evidenta clientilor
	activi pe server, lista de mesaje store va fi mereu nula.

	Am considerat 3 tipuri de mesaje, reprezentata sub forma de structuri:
	tcp_message:
	- primite de server de la un client tcp
	- contine informatii despre topicul la care vrea sa dea subscribe/
	unsubscribe, optiunea de sf si id-ul clientului.

	udp_message:
	- primite de server de la un client udp
	- formatul din enunt cu topic, tip de date si payload
	
	message_to_tcp:
	- trimis de server catre un destinatar client tcp
	- reprezinta mesajul format dupa "decodificarea" mesajului primit de server
	de la clientul udp

	
	
