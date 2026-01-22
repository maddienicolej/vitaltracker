from flask import Flask, render_template, request, redirect, session, url_for
import sqlite3
from flask_bcrypt import Bcrypt
from functools import wraps

app = Flask(__name__)
app.secret_key = "change_this_to_a_random_secret"
bcrypt = Bcrypt(app)


# ---- Database helper ----
def get_db():
    conn = sqlite3.connect("users.db")
    conn.row_factory = sqlite3.Row
    return conn

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
    return render_template("index.html")


# ---- Login ----
@app.route("/login", methods=["POST"])
def login():
    email = request.form["email"]
    password = request.form["password"]

    db = get_db()
    user = db.execute(
        "SELECT * FROM users WHERE email = ?",
        (email,)
    ).fetchone()

    if user and bcrypt.check_password_hash(user["password_hash"], password):
        session["user_id"] = user["id"]
        return redirect(url_for("home"))

    return redirect(url_for("index"))


# ---- Register ----
@app.route("/register", methods=["POST"])
def register():
    email = request.form["email"]
    password = request.form["password"]

    hashed_pw = bcrypt.generate_password_hash(password).decode("utf-8")

    db = get_db()
    try:
        db.execute(
            "INSERT INTO users (email, password_hash) VALUES (?, ?)",
            (email, hashed_pw)
        )
        db.commit()
    except sqlite3.IntegrityError:
        # Email already exists
        return "Email already exists"  # optionally redirect back with a flash message

    # Automatically log in the user after registration
    user = db.execute(
        "SELECT * FROM users WHERE email = ?",
        (email,)
    ).fetchone()

    session["user_id"] = user["id"]  # create session

    # Redirect to home
    return redirect(url_for("home"))


# ---- Dashboard Home Page ----
@app.route("/vitaltracker")
@login_required
def home():
    db = get_db()
    user = db.execute("SELECT * FROM users WHERE id = ?", (session["user_id"],)).fetchone()
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