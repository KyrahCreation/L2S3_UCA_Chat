# L2S3_UCA_Chat
Programmation et Systèmes - L2 S3 (UCA)

Instructions pour la compilation et l'exécution:

1. Compiler. Il y a un makefile à peu près bien foutu. Un simple 'make' suffira donc.
2. Lancer le serveur avec './server <port>'. Le port peut être n'importe quoi. J'effectuais mes tests avec 10020
3. Lancer un ou plusieurs clients avec './client <IP> <port>'. Le port est le même qu'à l'étape précédente, et l'adresse dépend de la machine où est exécutée le code. '127.0.0.1' histoire d'être sûr de ne pas se tromper en local.
4. Voilà. A priori ça fonctionne. 'make fclean' pour nettoyer le dossier et ne récupérer que les .c et le Makefile.

Commandes utiles (Client):
/l - pour afficher la liste des utilisateurs connectés
/q - pour se déconnecter

Commande utile (Serveur):
q - pour éteindre le serveur

