import { auth } from "./firebase.js"
import {
    createUserWithEmailAndPassword,
    signInWithEmailAndPassword, 
    onAuthStateChanged
} from "https://www.gstatic.com/firebasejs/12.8.0/firebase-auth.js";

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

onAuthStateChanged(auth, async (user) => {
    if (user) {
        const token = await user.getIdToken();

        await fetch("/sessionLogin", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ token })
        });
        window.location.href = "/vitaltracker";
    }
});