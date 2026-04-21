
import { auth } from "./firebase.js"
import { onAuthStateChanged, signOut} from "https://www.gstatic.com/firebasejs/12.8.0/firebase-auth.js";

console.log("JS LOADED");

document.addEventListener("DOMContentLoaded", () => {
    
    onAuthStateChanged(auth, (user) => {
        if (!user) {
            window.location.href = "/logout";
        }
    });

    const logout = document.getElementById('logout');
    logout.addEventListener("click", (e) => {
        console.log('List item was clicked');
        signOut(auth).then(() => {
        }).catch((error) => {
        const errorCode = error.code;
        const errorMessage = error.message;
        console.error("Account logout failed:", errorMessage);
        })
        
    });
});
