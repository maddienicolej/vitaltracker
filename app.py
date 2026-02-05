from flask import Flask, render_template, request, redirect, session, url_for
from functools import wraps
import firebase_admin
from firebase_admin import credentials, auth

cred = credentials.Certificate("key.json")
firebase_admin.initialize_app(cred)


app = Flask(__name__)
app.secret_key = "change_this_to_a_random_secret"


def login_required(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        if "user_id" not in session:
            return redirect(url_for("index"))
        return f(*args, **kwargs)
    return wrapper



# ---- Home (login + register page) ----
@app.route("/")
def index():
    if "user_id" in session:
        return redirect(url_for("home"))
    return render_template("auth.html")


@app.route("/sessionLogin", methods=["POST"])
def session_login():
    token = request.json["token"]

    decoded = auth.verify_id_token(token)
    session["user_id"] = decoded["uid"]

    return {"status": "ok"}


# ---- Dashboard Home Page ----
@app.route("/vitaltracker")
@login_required
def home():
    return render_template("home.html")

@app.route("/vitaltracker/ekg")
@login_required
def ekg():
    return render_template("ekg.html")

@app.route("/vitaltracker/temp")
@login_required
def temp():
    return render_template("temp.html")

@app.route("/vitaltracker/heart-rate")
@login_required
def hr():
    return render_template("hr.html")

@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("index"))



if __name__ == "__main__":
    app.run(debug=True)