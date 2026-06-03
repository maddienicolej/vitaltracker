# vitaltracker
Web App for Vital Tracker using HTML, CSS, C++, Python, and Flask

## Firebase Setup
1. Create a Firebase project.
2. Generate a Firebase Admin SDK service account key.
3. Download the JSON file.
4. Rename it to `key.json`.
5. Place it in the project root directory.

The application expects:

vitaltracker/
├── app.py
├── key.json
├── requirements.txt
├── esp32_to_firebase.cpp
└── templates/
    ├─ index.html
    ├─ home.html
    ├─ ekg.html
    ├─ hr.html
    └─ temp.html
└─ static/
    └─ css/
        └─ style.css
    └─ js/
        ├─ auth.js
        ├─ ekg.js
        ├─ firebase.js
        ├─ login.js
        └─ logout.js
    └─ img/
    

1. Create a Firebase project.
2. Enable Authentication.
3. Create a Realtime Database.
4. Download a service account key.
5. Rename it to key.json.
6. Update firebaseConfig.js with your project credentials. Rename to firebase.js.
7. Run the Flask application. (python app.py)