import { initializeApp } from "https://www.gstatic.com/firebasejs/12.8.0/firebase-app.js";
import { getAuth } from "https://www.gstatic.com/firebasejs/12.8.0/firebase-auth.js";

const firebaseConfig = initializeApp({
    apiKey: "AIzaSyCiTjrAgUaS7PY2DSuLZctDyGkvop839PQ",
    authDomain: "vitaltracker-a9d1c.firebaseapp.com",
    projectId: "vitaltracker-a9d1c",
    storageBucket: "vitaltracker-a9d1c.firebasestorage.app",
    messagingSenderId: "25609857288",
    appId: "1:25609857288:web:04e79bc8f3ce088e9fa4bd",
    measurementId: "G-N97LZYNTNP"
});

const auth = getAuth(firebaseConfig);

export { auth };