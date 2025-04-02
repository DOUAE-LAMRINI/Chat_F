# Smart Building with AI-Driven Support and IoT Systems (W.D.B)

## 📌 Description
W.D.B est un projet de fin d’études qui vise à développer un **immeuble intelligent** intégrant des **systèmes IoT et une intelligence artificielle** pour améliorer la sécurité, la gestion énergétique et le confort des habitants. Le projet repose sur une combinaison de **capteurs intelligents, microcontrôleurs et une application Web interactive**.

## 🏗️ Architecture du Smart Building
L’immeuble comporte **trois étages** (hors rez-de-chaussée), avec une maison intelligente par étage. Voici la répartition des technologies utilisées :

### 🔑 Systèmes d'accès et sécurité
- **Porte de chaque maison** : Arduino + Keypad + LCD I2C + Servo motor
- **Sécurité intérieure** : ESP32 + DHT11 (température et humidité) + Capteur de son
- **Capteur de mouvement** : Arduino + LED blanche + Bouton
- **Détection de gaz et incendie (cuisine)** : ESP32 + MQ135 + Capteur de flamme + LED rouge clignotante + Buzzer
- **Portail principal** : Arduino + MG811 (CO2) + TFT LCD
- **Ascenseur** : Arduino + RFID + Dot Matrix + Moteur

### 💡 Gestion de l’éclairage et du confort
- **Chambres** : LED blanche + Boutons (Arduino Uno)
- **Chambre de jeux** : RGB LED + Boutons (Arduino Uno)
- **Jardin** : ESP8266 + Capteur d'humidité du sol + Pompe d’arrosage automatique

### 🖥️ Application Web
L’application Web permet une interaction avec les systèmes du bâtiment et comprend les pages suivantes :
- **Home** : Introduction au projet
- **Guide** : FAQ et informations générales
- **Gallery** : Photos et images du bâtiment
- **Feedback** : Formulaire permettant aux utilisateurs de partager leurs avis
- **Syndicate** : Chatbot intelligent accessible uniquement aux résidents après authentification (Username, House Number, Email, Password)

Le **chatbot multilingue** est basé sur JSON et répond aux questions en **français, anglais et arabe**.

## 🛠️ Technologies utilisées
### 🔹 Backend
- Python
- SQL
- JSON
- CSV

### 🔹 Frontend
- HTML
- CSS
- Bootstrap
- JavaScript
- jQuery
- JSON

## 🚀 Installation & Exécution
1. **Cloner le projet**
   ```bash
   git clone https://github.com/ton-repo/Chat_F.git
   cd Chat_F
   ```
2. **Installer les dépendances backend**
   ```bash
   pip install -r requirements.txt
   ```
3. **Lancer le serveur**
   ```bash
   python app.py
   ```
4. **Accéder à l’application** : Ouvrir un navigateur et aller sur `http://localhost:5000`

## 👥 Équipe
- **Douae Lamrini** – Étudiante en Génie Informatique Embarquée
- **Wiam El-khanchoufi** – Étudiante en Génie Informatique Embarquée

---
🎯 **Projet réalisé dans le cadre du PFE à l'École Supérieure de Technologie d’Oujda (ESTO).**
