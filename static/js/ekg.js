import { db } from "./firebase.js";
import { ref, onValue, set } from "https://www.gstatic.com/firebasejs/12.8.0/firebase-database.js";

const dataRef = ref(db, 'readings/device001');
const statusRef = ref(db, 'status/device001');
//create buffer
let ekgBuffer = [];
const MAX_POINTS = 300; // adjust (200–500 is typical)
let timeIndex = 0;

onValue(statusRef, (snapshot) => {
  const flagdata = snapshot.val();
  const fallTime = flagdata.fallTime;
  const mpuLabel = new Date(fallTime * 1000);
  console.log(mpuLabel.toLocaleString());
  //show pop-up alert if fall is detected
  const mpuAlert = document.getElementById('mpu-popup');
  if (flagdata.mpuBool == true) {
    mpuAlert.style.display = 'block';
    document.getElementById("mpuTime").textContent = "The Vital Tracker has detected a fall from your patient at " + mpuLabel.toLocaleString() + ". Attend to their room immediately."
  }
  
  document.getElementById('mpuBtn').onclick = () => {
    mpuAlert.style.display = 'none';
    //update bool to false in database after closing pop-up
    set(statusRef, {
      tempBool: false,
      mpuBool: false
    });
  };

  //show pop-alert if temperature spikes
  const tempTime = flagdata.tempTime;
  const tempLabel = new Date(tempTime * 1000);
  const label=tempLabel.toLocaleString();
  const tempAlert = document.getElementById('temp-popup');
  if (flagdata.tempBool == true) {
    tempAlert.style.display = 'block';
    document.getElementById("tempTime").textContent = "The Vital Tracker has detected a spike in temperature at " + label + ". Attend to their room immediately.";
  }
  document.getElementById('tempBtn').onclick = () => {
    tempAlert.style.display = 'none';
    //update bool to false in database after closing pop-up
    set(statusRef, {
      tempBool: false,
      mpuBool: false
    });
  };

});

onValue(dataRef, (snapshot) => {
  const data = snapshot.val();
  console.log(data);

   if (!data) {
      console.log("No data available");
      return;
    }

  const latestTimestamp = Object.keys(data).pop();
  const latestReading = data[latestTimestamp];
  const ekgPoints = latestReading.ekg.split(',').map(Number);

  const formattedEKG = ekgPoints.map((value, index) => ({
    x: index,
    y: value
  }));


  //display values from each sensor
  document.getElementById("temp").textContent = latestReading.temperature + "°F";
  //document.getElementById("spo2").textContent = latestReading.spo2 + "%";
  document.getElementById("bpm").textContent = latestReading.bpm + " BPM";
  
  // collect ekg points and update graph
  const newPoints = latestReading.ekg
  .split(',')
  .map(Number);

// Add new data
newPoints.forEach(value => {
  ekgBuffer.push({ x: timeIndex++, y: value });
});

// Trim old data
if (ekgBuffer.length > MAX_POINTS) {
  ekgBuffer.splice(0, ekgBuffer.length - MAX_POINTS);
}
  chart.data.datasets[0].data = ekgBuffer;
  chart.update();
});


let chart;
const ctx = document.getElementById('myChart');
chart = new Chart(ctx, {
  type: 'line',
  data: {
  datasets: [{
    label: 'EKG Analog Points',
    data: [],
    //data: formattedEKG,
    borderColor: '#ff0000', // neon green
    backgroundColor: 'white',
    borderWidth: 1,
    pointRadius: 0,
    tension: 0
    }]
  },
  options: {
  animation: false,
  responsive: true,
  maintainAspectRatio: false,
  scales: {
    x: {
      type: 'linear',
      title: {
      display: true,
      text: 'Time'
      }
    },
    y: {
      title: {
      display: true,
      text: 'Amplitude'
      }
    }
  }
}
});