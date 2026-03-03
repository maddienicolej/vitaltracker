
import { auth } from "./firebase.js"
import { onAuthStateChanged, signOut} from "https://www.gstatic.com/firebasejs/12.8.0/firebase-auth.js";

console.log("JS LOADED");

document.addEventListener("DOMContentLoaded", () => {
    
    onAuthStateChanged(auth, (user) => {
        if (!user) {
            window.location.href = "/logout";
        }
    });

    const dropdown = document.getElementById('drop-options');
    dropdown.addEventListener("click", (e) => {
        const item = e.target.closest(".item");
        if (!item) return;

        e.preventDefault();

        if (item.dataset.value === "logout") {
        signOut(auth).then(() => {
        }).catch((error) => {
        const errorCode = error.code;
        const errorMessage = error.message;
        console.error("Account logout failed:", errorMessage);
        })
        }     
    });
});
