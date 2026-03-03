import { auth, provider } from "./firebase.js"
import {
    createUserWithEmailAndPassword,
    signInWithEmailAndPassword, 
    onAuthStateChanged,
    signInWithPopup,
    GoogleAuthProvider
} from "https://www.gstatic.com/firebasejs/12.8.0/firebase-auth.js";

console.log("JS LOADED");

document.addEventListener("DOMContentLoaded", () => {
    //show login register form
        const showForm = (formId) => {
            document.querySelectorAll(".form-box").forEach(form => form.classList.remove("active"));
            document.getElementById(formId).classList.add("active");
        };
        document.getElementById("to-login").addEventListener("click", (e) => {
            e.preventDefault();
            showForm("login-form");
        });
        document.getElementById("to-register").addEventListener("click", (e) => {
            e.preventDefault();
            showForm("register-form")
        });
            
        //process registering
        const registerForm = document.querySelector('#register')
        registerForm.addEventListener('submit', (e) => {
            e.preventDefault();
            //get user info
            const email = registerForm['register-email'].value;
            const password = registerForm['register-password'].value;

            createUserWithEmailAndPassword(auth, email, password).then((userCredential) => {
            const user = userCredential.user;
            console.log("Account registration successful, navigating to home page");
        })
        .catch((error) => {
            // Handle errors here
            const errorCode = error.code;
            const errorMessage = error.message;
            console.error("Account registration failed:", errorMessage);
        });
        });

        //process login
        const loginForm = document.querySelector('#login');
        loginForm.addEventListener('submit', (e) => {
            e.preventDefault();

            const email = loginForm['login-email'].value;
            const password = loginForm['login-password'].value;

            signInWithEmailAndPassword(auth, email, password)
        .then((userCredential) => {
            console.log(userCredential)
        });
        });

        //sign in with google
        const googleLogin = document.getElementById("google-login-btn")
        googleLogin.addEventListener("click", async() => {
            try{
                await signInWithPopup(auth, provider);
            } catch (error) {
            const errorCode = error.code;
            const errorMessage = error.message;
            const email = error.customData.email;
            const credential = GoogleAuthProvider.credentialFromError(error);
            }
        });

        //move to home page if account found
        onAuthStateChanged(auth, async (user) => {
            if (user) {
                try {
                    const token = await user.getIdToken();

                    const response = await fetch("/sessionLogin", {
                        method: "POST",
                        headers: { "Content-Type": "application/json" },
                        body: JSON.stringify({ token })
                    });

                    if (response.ok) {
                        window.location.href = "/vitaltracker";
                    } else {
                        console.error("Session login failed");
                    }
                } catch (error) {
                    console.error("Session setup error:", error);
                }
            }
        });

    
});