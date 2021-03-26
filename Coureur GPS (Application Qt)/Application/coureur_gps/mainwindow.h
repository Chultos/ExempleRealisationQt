#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QPainter>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void envoiRequete();
    void gerer_donnees();
    void afficher_erreur(QAbstractSocket::SocketError);
    void on_recuperation_toggled(bool checked);
    void on_lancementArret_toggled(bool checked);
    void decodageTrame();
    float getLongitude();
    float getLatitude();
    void traceCoureur();
    float getAltitude();
    void courbeAltitude();
    int getFrequenceCardiaque();
    void courbeFrequenceCardiaque();
    int getTempsEcoule();
    float getDistanceParcourue(float latitude_depart, float longitude_depart, float latitude_arrivee, float longitude_arrivee);
    float getVitesse();
    float getCaloriesDepensees();
    int getNombreSatellites();
    void on_pushButton_changementCarte_toggled(bool checked);
    void sauvegardeBdd();
    void verifConnexion();
    void verifChecksum(QString trame);

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    QTimer *pTimer;
    QImage *pCarte;
    QImage *pCarteCoureur;
    QImage *pSignalSatellite;
    QString cheminLogoSignal;
    QString cheminImageCarte;

    QSqlDatabase bdd;
    QByteArray reponse;
    bool valeursDeDepartRecues;
    QStringList liste;

    int lastPx;
    int lastPy;
    int px;
    int py;
    const float latHG = 46.173311;
    const float lonHG = -1.195703;
    const float latBD = 46.135451;
    const float lonBD = -1.136125;

    int courbeCount;
    bool rectangleStyle;

    float latA;
    float latitude;
    float lonA;
    float longitude;
    float altitude;

    float frequenceCardiaque;
    float frequenceCardiaqueMax;
    float intensiteEffort;

    int heureDeDepart;
    int minuteDeDepart;
    int secondeDeDepart;
    int tempsDepart;
    int dernierTempsEcoule;

    int tempsEcoule;
    float distanceParcourue;
    float derniereDistanceParcourue;
    float vitesse;
    int caloriesDepensees;

    int age;
    int taille;
    float poids;
};

#endif // MAINWINDOW_H
