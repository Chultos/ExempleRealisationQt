"""
Serveur GPS distribuant des trames NMEA sur un réseau TCP/IP
@author David SALLE (pour la section STS SNIR de l'ICSSA)
@date 03/06/2020
@version 0.1
@licence GPLv3

Ce script permet de lire tout un ensemble de trames NMEA stockées dans un fichier texte
pour les envoyer une par une à des clients TCP.
Cela permet de simuler la connexion à un GPS en TCP au lieu de RS232
"""

# Les bibliothèques utilisées
import sys
import os
import socket
import threading
import signal
import string
import time


# Variable partagée par les 2 threads
laTramePartagee = "..."


class ThreadFichier(threading.Thread) :
    """Classe pour gérer la lecture des trames NMEA dans le fichier texte
    """

    def __init__(self, nom_fichier) :
        """Méthode constructeur pour initialiser l'objet
        @param nom_fichier contenant les trames NMEA
        @return rien
        """

        # Lecture du fichier contenant les trames NMEA
        print("Thread FICHIER    => Ouverture de ", nom_fichier)
        self.fichier_nmea = open(nom_fichier, "r")
        self.lignes  = self.fichier_nmea.readlines()
        self.fichier_nmea.close()

        # Modification des trames pour insérer le CR avant LF
        #print("Thread FICHIER    => Modification du CR+LF")
        #for ligne in self.lignes:
        #    resultat = ligne.replace("\n", "\r\n")
        #    ligne = resultat


        """
        print("Thread FICHIER    => ", self.lignes[0])
        print("Thread FICHIER    => ", self.lignes[1])
        print("Thread FICHIER    => ", self.lignes[2])
        """

        # Initialisation de la thread
        threading.Thread.__init__ (self)


    def run(self) :
        """Méthode surchargée pour choisir toutes les secondes une nouvelle trame à envoyer
        @param aucun
        @return rien
        """

        # Declaration de la variable globale partagee
        global laTramePartagee

        # Boucle de lecture (infinie)
        compteur = 0
        while compteur < len(self.lignes):
            laTramePartagee = self.lignes[compteur]
            compteur = compteur + 1

            # Dès qu'on approche de la fin du tableau on reboucle
            if compteur >= ( len(self.lignes) - 1):
                compteur = 0

            # Attente d'une seconde pour simuler un vrai coureur (plus rapide que normal)
            time.sleep(1)



class ThreadReseau(threading.Thread) :
    """Classe pour gerer les clients TCP qui veulent récupérer les trames NMEA
    """

    def __init__(self, channel, details) :
        """Méthode constructeur
        @param channel
        @param details
        @return rien
        """

        # Ouverture du port RS232
        self.channel = channel
        self.details = details
        threading.Thread.__init__ (self)


    def run(self) :
        """Méthode surchargée pour répondre aux clients TCP
        @param aucun
        @return rien

        Le client doit se connecter et envoyer "RETR\r\n"
        le serveur retourne la dernière trame NMEA choisie par ThreadFichier
        """

        # Déclaration de la variable globale partagée
        global laTramePartagee

        # Affichage des détails de connexion
        print("Thread RESEAU     => Nouvelle connexion : ", self.details[0])

        # Boucle de lecture et d'envoi
        while True:
            try :
                reception = self.channel.recv(1024)
                print("Thread RESEAU     => Reçu de", self.details[0], ": ", reception, len(reception) )
                if (	len(reception) == 6 and
						reception[0] == 0x52 and
						reception[1] == 0x45 and
						reception[2] == 0x54 and
						reception[3] == 0x52 and
						reception[4] == 0x0d and
						reception[5] == 0x0a) :
                    self.channel.send(bytes(laTramePartagee, 'UTF-8'))
                else :
                    self.channel.send(bytes("?\r\n", 'UTF-8'))
            except :
                # Fermeture de la connexion
                self.channel.close()
                print("Thread RESEAU     => Déconnexion de", self.details[0])
                break



# Point d'entrée du script
if __name__ == "__main__":

    # Bannière d'accueil
    print("**********************")
    print("** Serveur GPS v0.2 **")
    print("**********************")

    # Paramètres de l'application
    fichier_nmea = "marathon_la_rochelle_30112008_nmea.txt"
    #fichier_nmea = "marathon_la_rochelle_temp.txt"
    port_tcp = 1664

    # Pour sortir normalement et proprement avec un CTRL+C
    signal.signal(signal.SIGINT, signal.default_int_handler)

    try :
        # Lancement de la thread FICHIER
        ThreadFichier(fichier_nmea).start()

        # Création de la socket serveur
        socketServeur = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Attachement de la socket
        socketServeur.bind(("", port_tcp))

        # Création d'une file d'attente de clients
        socketServeur.listen(30)

        # Boucle d'écoute
        print("Thread PRINCIPALE => Serveur en écoute sur ", port_tcp)
        while (True) :
            channel, details = socketServeur.accept()
            ThreadReseau(channel, details).start()

    except KeyboardInterrupt :
        print("Au revoir ...")