import { initializeApp } from "https://www.gstatic.com/firebasejs/12.8.0/firebase-app.js";
import { getAuth, GoogleAuthProvider } from "https://www.gstatic.com/firebasejs/12.8.0/firebase-auth.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/12.8.0/firebase-database.js";

const firebaseConfig = {
    apiKey: "AIzaSyCiTjrAgUaS7PY2DSuLZctDyGkvop839PQ",
    authDomain: "vitaltracker-a9d1c.firebaseapp.com",
    projectId: "vitaltracker-a9d1c",
    storageBucket: "vitaltracker-a9d1c.firebasestorage.app",
    messagingSenderId: "25609857288",
    appId: "1:25609857288:web:04e79bc8f3ce088e9fa4bd",
    measurementId: "G-N97LZYNTNP"
};

const app = initializeApp(firebaseConfig);

const auth = getAuth(app);
const provider = new GoogleAuthProvider();
const db = getDatabase(app);
export { app, auth, provider, db};


