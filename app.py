from flask import Flask, render_template, request, redirect, session, url_for
from functools import wraps
import firebase_admin
from firebase_admin import credentials, auth


# ---- Firebase Setup ----
cred = credentials.Certificate("key.json")
firebase_admin.initialize_app(cred)


app = Flask(__name__)
app.secret_key = "change_this_to_a_random_secret"


# ---- Login Required Decorator ----
def login_required(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        if "user_id" not in session:
            return redirect(url_for("landing"))
        return f(*args, **kwargs)
    return wrapper


# ---- Public Landing Page ----
@app.route("/")
def landing():
    if "user_id" in session:
        return redirect(url_for("dashboard"))
    return render_template("startup.html")


# ---- Firebase Session Bridge ----
@app.route("/sessionLogin", methods=["POST"])
def session_login():
    try:
        token = request.json["token"]
        decoded = auth.verify_id_token(token)
        session["user_id"] = decoded["uid"]
        return {"status": "ok"}
    except Exception:
        return {"error": "Invalid token"}, 401
    
@app.route("/login")
def login_page():
    return render_template("login.html")



# ---- Protected Dashboard Routes ----
@app.route("/vitaltracker")
@login_required
def dashboard():
    return render_template("home.html")


@app.route("/vitaltracker/ekg")
@login_required
def ekg():
    return render_template("ekg.html")


# ---- Logout ----
@app.route("/logout")
@login_required
def logout():
    session.clear()
    return redirect(url_for("landing"))


if __name__ == "__main__":
    app.run(debug=True)
