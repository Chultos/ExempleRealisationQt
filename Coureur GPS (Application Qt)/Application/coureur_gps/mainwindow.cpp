#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initialisation de l'interface graphique
    ui->setupUi(this);

    cheminImageCarte = ":/cartes/images/carte_la_rochelle_plan.png";
    // Instanciation de l'image de la carte
    pCarte = new QImage();
    // Chargement depuis un fichier
    pCarte->load(cheminImageCarte);
    // Affichage dans un QLabel, ici label_carte
    ui->label_Carte->setPixmap(QPixmap::fromImage(*pCarte));

    //Définis l'image a afficher pour le signal satellite
    cheminLogoSignal = ":/signalSatellite/images/signalLogo/noSignal.png";
    // Instanciation de l'image du logo signal
    pSignalSatellite = new QImage();
    // Chargement depuis un fichier
    pSignalSatellite->load(cheminLogoSignal);
    // Affichage dans un QLabel
    ui->label_signalSatellite->setPixmap(QPixmap::fromImage(*pSignalSatellite));

    // Instanciation de la socket
    tcpSocket = new QTcpSocket(this);
    // Attachement d'un slot qui sera appelé à chaque fois que des données arrivent (mode asynchrone)
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(gerer_donnees()));
    // Idem pour les erreurs
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(afficher_erreur(QAbstractSocket::SocketError)));
    //Idem pour vérifier la connexion
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(verifConnexion()));

    //Instanciation d'un timer
    pTimer = new QTimer();
    // Lancement du timer avec un tick toutes les 1s
    pTimer->start(1000);

    //Préparation de quelques variables
    valeursDeDepartRecues = false;
    rectangleStyle = false;

    // "Connexion" à la base de données SQLite
    bdd = QSqlDatabase::addDatabase("QSQLITE");
    bdd.setDatabaseName("D:\\DEV\\Cours\\Qt\\coureur_gps\\ma_bdd.sqlite");
    if (!bdd.open())
    {
     qDebug() << "Erreur: Problème de connexion avec la BDD";
    }
    else
    {
     qDebug() << "BDD: Connexion établie";
    }
}

MainWindow::~MainWindow()
{
    // Destruction du timer
    pTimer->stop();
    delete pTimer;

    // Destruction de la socket
    tcpSocket->abort();
    delete tcpSocket;

    // Destruction de l'interface graphique
    delete ui;
}

void MainWindow::envoiRequete()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
}

void MainWindow::gerer_donnees()
{
    // Réception des données
    reponse = tcpSocket->readAll();

    // Affichage
    ui->lineEdit_reponse->setText(QString(reponse));

    age = ui->lineEdit_Age->text().toInt();
    taille = ui->lineEdit_Taille->text().toInt();
    poids = ui->lineEdit_Poids->text().toFloat();

    frequenceCardiaqueMax = 220 - age;
    ui->lineEdit_FrequenceCardiaqueMax->setText(QString::number(frequenceCardiaqueMax));
    decodageTrame();
    sauvegardeBdd();
}

void MainWindow::afficher_erreur(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
            //Reset de l'état des boutons pour ne pas boucler l'erreur
            ui->recuperation->setChecked(false);
            ui->lancementArret->setChecked(false);
            break;
        case QAbstractSocket::HostNotFoundError:
            //Reset de l'état des boutons pour ne pas boucler l'erreur
            ui->recuperation->setChecked(false);
            ui->lancementArret->setChecked(false);
            //Message d'erreur
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Hôte introuvable"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            //Reset de l'état des boutons pour ne pas boucler l'erreur
            ui->recuperation->setChecked(false);
            ui->lancementArret->setChecked(false);
            //Message d'erreur
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Connexion refusée"));
            break;
        default:
            //Reset de l'état des boutons pour ne pas boucler l'erreur
            ui->recuperation->setChecked(false);
            ui->lancementArret->setChecked(false);
            //Message d'erreur
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Erreur : %1.")
                                     .arg(tcpSocket->errorString()));
    }
}

void MainWindow::on_recuperation_toggled(bool checked)
{
    if(checked == true) {
        // Récupération des données 1 fois par seconde
        connect(pTimer, SIGNAL(timeout()), this, SLOT(envoiRequete()));
    } else {
        disconnect(pTimer, SIGNAL(timeout()), this, SLOT(envoiRequete()));

        //CHANGE L'ICONE DE SIGNAL SATELLITE
        //Définis l'image a afficher pour le signal satellite
        cheminLogoSignal = ":/signalSatellite/images/signalLogo/noSignal.png";
        // Chargement depuis un fichier
        pSignalSatellite->load(cheminLogoSignal);
        // Affichage dans un QLabel
        ui->label_signalSatellite->setPixmap(QPixmap::fromImage(*pSignalSatellite));
    }
}

void MainWindow::on_lancementArret_toggled(bool checked)
{
    if(checked == true) {
        //Empeche l'utilisateur de modifier l'ip et le port pour éviter des bugs
        ui->lineEdit_ip->setReadOnly(true);
        ui->lineEdit_port->setReadOnly(true);

        // Récupération des paramètres
        QString adresse_ip = ui->lineEdit_ip->text();
        unsigned short port_tcp = ui->lineEdit_port->text().toInt();

        // Connexion au serveur
        tcpSocket->connectToHost(adresse_ip, port_tcp);

    } else {
        // Déconnexion du serveur
        tcpSocket->close();

        //Arret de la récupération des données
        ui->recuperation->setChecked(false);

        //Empeche l'utilisateur de lancer la récupération des données sans etre connecté
        ui->recuperation->setCheckable(false);

        //L'utilisateur peut de nouveau modifier l'ip et le port
        ui->lineEdit_ip->setReadOnly(false);
        ui->lineEdit_port->setReadOnly(false);
    }
}

//Empeche l'utilisateur de lancer la récupération des données sans etre connecté
void MainWindow::verifConnexion() {
    ui->recuperation->setCheckable(true);
}

void MainWindow::verifChecksum(QString trame) {

    char somme = 0;
    int checksum = 0;
    QStringList listeChecksum;

    //Sépare le checksum et le $ de la trame d'origine pour calculer le checksum
    listeChecksum = trame.split("*");
    QString trameChecksum = listeChecksum[0].mid(1,listeChecksum[0].size());

    //Calcul du checksum
    for(int i=0; i<trameChecksum.length(); i++) {
        somme = char(somme ^ trameChecksum[i].toLatin1());
    }
    checksum = somme % 256;
    //Conversion en hexadécimal
    QString result = QString::number(checksum, 16);

    //Récupération du checksum de la trame d'origine
    liste = trame.split(",");
    QString trameChecksumValue = liste[14].mid(5, 2);

    //Comparaison des deux checksums et affichage d'un message d'erreur si il y en a une
    if(trameChecksumValue != result.toLower()) {
        ui->label_Checksum->setText("Erreur: Checksum invalide");
    } else {
        ui->label_Checksum->setText("");
    }
}

float MainWindow::getLatitude() {
    int degresLa = liste[2].mid(0,2).toInt();
    float minutesLa = liste[2].mid(2,7).toFloat();
    latitude = degresLa + (minutesLa / 60);

    if (liste[3] == "S") {
        latitude = latitude * -1;
    }
    return latitude;
}

float MainWindow::getLongitude() {
    int degresLo = liste[4].mid(0,3).toInt();
    float minutesLo = liste[4].mid(3,7).toFloat();
    longitude = degresLo + (minutesLo / 60);

    if (liste[5] == "W") {
        longitude = longitude * -1;
    }

    //Permet de récupérer certaines valeurs 1 fois au départ de l'application
    if(valeursDeDepartRecues == false) {
        latA = latitude;
        lonA = longitude;
    }
    return longitude;
}

float MainWindow::getAltitude() {
    altitude = liste[9].toFloat();
    return altitude;
}

int MainWindow::getFrequenceCardiaque() {
    frequenceCardiaque = liste[14].mid(0, 4).toInt();
    return frequenceCardiaque;
}

int MainWindow::getTempsEcoule() {
    //Permet de récupérer certaines valeurs 1 fois au départ de l'application
    if(valeursDeDepartRecues == false) {
        heureDeDepart = liste[1].mid(0,2).toInt();
        minuteDeDepart = liste[1].mid(2,2).toInt();
        secondeDeDepart = liste[1].mid(4,2).toInt();

        tempsDepart = ((3600 * heureDeDepart) + (60 * minuteDeDepart) + secondeDeDepart);
        valeursDeDepartRecues = true;
    }

    int heures = liste[1].mid(0,2).toInt();
    int minutes = liste[1].mid(2,2).toInt();
    int secondes = liste[1].mid(4,2).toInt();

    tempsEcoule = ((3600 * heures) + (60 * minutes) + secondes) - tempsDepart;
    return tempsEcoule;
}

float MainWindow::getDistanceParcourue(float latitude_depart, float longitude_depart, float latitude_arrivee, float longitude_arrivee) {
    latitude_depart = latitude_depart * (M_PI/180);
    longitude_depart = longitude_depart * (M_PI/180);
    latitude_arrivee = latitude_arrivee * (M_PI/180);
    longitude_arrivee = longitude_arrivee * (M_PI/180);

    distanceParcourue += 6378 * qAcos( qSin(latitude_depart)*qSin(latitude_arrivee) + qCos(latitude_depart) * qCos(latitude_arrivee) * qCos(longitude_depart-longitude_arrivee));
    return distanceParcourue;
}

float MainWindow::getVitesse() {
    vitesse = ((distanceParcourue -  derniereDistanceParcourue) / (tempsEcoule - dernierTempsEcoule))*3600;
    return vitesse;
}

float MainWindow::getCaloriesDepensees() {
    caloriesDepensees = distanceParcourue * poids * 1.036;
    return caloriesDepensees;
}

void MainWindow::traceCoureur() {
    lastPx = 694 * ((lonA - lonHG) / (lonBD - lonHG));
    lastPy = 638 * (1.0 - (latA - latBD) / (latHG - latBD));
    px = 694 * ((longitude - lonHG) / (lonBD - lonHG));
    py = 638 * (1.0 - (latitude - latBD) / (latHG - latBD));
    // Préparation du contexte de dessin sur une image existante
    QPainter c(pCarte);
    // Choix de la couleur
    c.setPen(Qt::red);
    // Dessin d'une ligne
    c.drawLine(lastPx, lastPy, px, py);
    // Fin du dessin et application sur l'image
    c.end();
    ui->label_Carte->setPixmap(QPixmap::fromImage(*pCarte));
}

void MainWindow::courbeAltitude() {
    QPainter cAlt(pCarte);
    cAlt.setPen(Qt::gray);
    if (rectangleStyle == false) {
        //Affiche l'emplacement des courbes
        cAlt.fillRect(41, 579, 201, 52, QColor(191, 217, 255));
    }
    if(courbeCount < 200) {
        cAlt.drawLine(courbeCount + 42, 627, courbeCount + 42, 627 - (altitude*2));
    } else {
        //Efface les anciennes lignes
        cAlt.fillRect(41, 579, 201, 52, QColor(191, 217, 255));
    }

    // Fin du dessin et application sur l'image
    cAlt.end();
}

void MainWindow::courbeFrequenceCardiaque() {
    QPainter cFC(pCarte);
    cFC.setPen(Qt::red);
    if (rectangleStyle == false) {
        //Affiche l'emplacement des courbes
        cFC.fillRect(41, 523, 201, 50, QColor(191, 217, 255));
        rectangleStyle = true;
    }
    if(courbeCount < 200) {
        cFC.drawLine(courbeCount + 42, 572, courbeCount + 42, 572 - (frequenceCardiaque/5));
        courbeCount++;
    } else {
        //Efface les anciennes lignes
        cFC.fillRect(41, 523, 201, 50, QColor(191, 217, 255));
        courbeCount = 0;
    }
    // Fin du dessin et application sur l'image
    cFC.end();
    ui->label_Carte->setPixmap(QPixmap::fromImage(*pCarte));
}

int MainWindow::getNombreSatellites() {
    int nombreSatellites = liste[7].toInt();
    return nombreSatellites;
}

void MainWindow::decodageTrame()
{
    QString reponseTrame = reponse;
    verifChecksum(reponseTrame);
    liste = reponseTrame.split(",");

    //NOMBRE DE SATELLITES
    //Affichage du résultat dans le lineEdit

    if (getNombreSatellites() >= 4) {
        cheminLogoSignal = ":/signalSatellite/images/signalLogo/perfectSignal.png";
    }
    else if (getNombreSatellites() == 3) {
        cheminLogoSignal = ":/signalSatellite/images/signalLogo/goodSignal.png";
    } else {
        cheminLogoSignal = ":/signalSatellite/images/signalLogo/badSignal.png";
    }

    // Chargement depuis un fichier
    pSignalSatellite->load(cheminLogoSignal);
    // Affichage dans un QLabel
    ui->label_signalSatellite->setPixmap(QPixmap::fromImage(*pSignalSatellite));

    //LATITUDE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_Latitude->setText(QString::number(getLatitude()));

    //LONGITUDE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_Longitude->setText(QString::number(getLongitude()));

    //DESSIN DU COUREUR SUR LA CARTE
    traceCoureur();

    //ALTITUDE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_Altitude->setText(QString::number(getAltitude()));

    //DESSIN DE LA COURBE D'ALTITUDE
    courbeAltitude();

    //FREQUENCE CARDIAQUE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_FrequenceCardiaque->setText(QString::number(getFrequenceCardiaque()));

    //DESSIN DE LA COURBE FREQUENCE CARDIAQUE
    courbeFrequenceCardiaque();

    //INTENSITE EFFORT
    intensiteEffort = (frequenceCardiaque / frequenceCardiaqueMax)*100;
    //Affichage du résultat sur la progressBar
    ui->progressBar_IntesiteEffort->setValue(intensiteEffort);

    //TEMPS ECOULE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_TempsEcoule->setText(QString::number(getTempsEcoule()));

    //DISTANCE PARCOURUE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_DistanceParcourue->setText(QString::number(getDistanceParcourue(latA, lonA, latitude, longitude)));

    //Actualisation des valeurs
    latA = getLatitude();
    lonA = getLongitude();

    //Permet de récupérer certaines valeurs 1 fois au départ de l'application
    if(valeursDeDepartRecues == false) {
        derniereDistanceParcourue = getDistanceParcourue(latA, lonA, latitude, longitude);
        dernierTempsEcoule = getTempsEcoule();
        valeursDeDepartRecues = true;
    }
    //VITESSE
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_Vitesse->setText(QString::number(getVitesse()));


    //CALORIES DEPENSEES
    //Affichage du résultat dans le lineEdit
    ui->lineEdit_Calories->setText(QString::number(getCaloriesDepensees()));
}

void MainWindow::on_pushButton_changementCarte_toggled(bool checked)
{
    if(checked == true) {
        cheminImageCarte = ":/cartes/images/carte_la_rochelle_satellite.png";
        // Chargement depuis un fichier
        pCarte->load(cheminImageCarte);
        // Affichage dans un QLabel, ici label_carte
        ui->label_Carte->setPixmap(QPixmap::fromImage(*pCarte));
        //Permet de ré-afficher les emplacements de courbe
        rectangleStyle = false;
    } else {
        cheminImageCarte = ":/cartes/images/carte_la_rochelle_plan.png";
        // Chargement depuis un fichier
        pCarte->load(cheminImageCarte);
        // Affichage dans un QLabel, ici label_carte
        ui->label_Carte->setPixmap(QPixmap::fromImage(*pCarte));
        //Permet de ré-afficher les emplacements de courbe
        rectangleStyle = false;
    }
}

void MainWindow::sauvegardeBdd() {
    QSqlQuery query;
    query.prepare("INSERT INTO donneesCoureur (latitude, longitude, altitude, frequenceCardiaque, tempsEcoule, distanceParcourue, vitesse, caloriesDepensees, age, taille, poids)"
                  "VALUES (:latitude, :longitude, :altitude, :frequenceCardiaque, :tempsEcoule, :distanceParcourue, :vitesse, :caloriesDepensees, :age, :taille, :poids)");
    query.bindValue(":latitude", latitude);
    query.bindValue(":longitude", longitude);
    query.bindValue(":altitude", altitude);
    query.bindValue(":frequenceCardiaque", frequenceCardiaque);
    query.bindValue(":tempsEcoule", tempsEcoule);
    query.bindValue(":distanceParcourue", distanceParcourue);
    query.bindValue(":vitesse", vitesse);
    query.bindValue(":caloriesDepensees", caloriesDepensees);
    query.bindValue(":age", age);
    query.bindValue(":taille", taille);
    query.bindValue(":poids", poids);

    if(!query.exec())
    {
     qDebug() << query.lastError().text() << "- Ne pas oublier de changer le chemin de la bdd ligne 48";
    }
}
