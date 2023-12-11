const firebaseConfig = {
  apiKey: "AIzaSyAjZN7E2KzqD6eoNTBBNvXmxYmplxWOYZg",
  authDomain: "fishfeeder-8bf7c.firebaseapp.com",
  databaseURL: "https://fishfeeder-8bf7c-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "fishfeeder-8bf7c",
  storageBucket: "fishfeeder-8bf7c.appspot.com",
  messagingSenderId: "501285241648",
  appId: "1:501285241648:web:9fb7f3c61a4db520cb3ef5",
  measurementId: "G-69HGD1SKNP"
};



// Initialize Firebase
firebase.initializeApp(firebaseConfig);
const db = firebase.firestore();

$(document).ready(function () {
  var database = firebase.database();
  var Led1Status;
  var PrimerButton;
  var ManualStatus;

  // Real-time listener for scheduler settings
  var schedulerRef;


  
  // Real-time listener for Led1Status
  database.ref("Led1Status").on("value", function (snapshot) {
    Led1Status = snapshot.val();
    updateLedStatusUI();
  });

  database.ref("ManualStatus").on("value", function (snapshot) {
    ManualStatus = snapshot.val();
    updateLedStatusUI();
  });

  database.ref("PrimerButton").on("value", function (snapshot) {
    PrimerButton = snapshot.val();
    updateLedStatusUI();
  });



// ...

// ...
// Real-time listener for scheduler settings
database.ref("schedulerSettings").on("value", function (snapshot) {
  const schedulerSettings = snapshot.val();

  // Check if schedulerSettings is not null before displaying
  if (schedulerSettings !== null) {
    displaySchedulerSettings(schedulerSettings);
    displayRunningTime(schedulerSettings); // Add this line to display running time
  } else {
    // Handle the case when schedulerSettings is null
    displaySchedulerSettings(null);
  }
});

// Function to update UI based on scheduler settings
function displaySchedulerSettings(settings) {
  const displayElement = document.getElementById("displaySchedulerSettings");

  if (settings) {
    const { hour, minute, ampm, days, amountToFeed } = settings;

    const formattedSettings = `Hour: ${hour}, Minute: ${minute}, AM/PM: ${ampm}, Days: ${days.join(", ")}, Amount to Feed: ${amountToFeed}`;

    displayElement.textContent = formattedSettings;
  } else {
    displayElement.textContent = "No scheduler settings available.";
  }
}





// Real-time listener for currentTime
setInterval(function () {
  const now = new Date();
  const currentHour = now.getHours();
  const currentMinute = now.getMinutes();
  const currentAmPm = currentHour >= 12 ? "pm" : "am";
  const formattedCurrentTime = {
    hour: currentHour % 12 || 12,
    minute: currentMinute.toString().padStart(2, '0'),
    ampm: currentAmPm,
    day: now.toLocaleDateString("en-US", { weekday: "short" }).toUpperCase()
  };

  displayCurrentTime(formattedCurrentTime);
}, 1000);

// Function to update UI based on current time
function displayCurrentTime(currentTime) {
  const displayRunningTimeElement = document.getElementById("displayRunningTime");

  if (currentTime) {
    const { hour, minute, ampm, day } = currentTime;

    const formattedTime = `Current Time: Hour: ${hour}, Minute: ${minute}, AM/PM: ${ampm}, Day: ${day}`;

    displayRunningTimeElement.textContent = formattedTime;
  } else {
    displayRunningTimeElement.textContent = "No current time available.";
  }


}

// Inside the setInterval function
setInterval(function () {
  const now = new Date();
  const currentHour = now.getHours();
  const currentMinute = now.getMinutes();
  const currentAmPm = currentHour >= 12 ? "pm" : "am";
  const formattedCurrentTime = {
    hour: currentHour % 12 || 12,
    minute: currentMinute.toString().padStart(2, '0'),
    ampm: currentAmPm,
    day: now.toLocaleDateString("en-US", { weekday: "short" }).toUpperCase()
  };

  displayCurrentTime(formattedCurrentTime);

  // Send the current time to the real-time database
  const currentTimeRef = database.ref("currentTime");
  currentTimeRef.set(formattedCurrentTime);
}, 1000);









// Event listener for the "SAVE" button click in the scheduler
document.getElementById("saveButton").addEventListener("click", function () {
  saveSchedulerSettings();
});




// Function to save scheduler settings to Firebase
function saveSchedulerSettings() {
  let selectedHour = parseInt(document.getElementById("hour-dd").value, 10); // Convert to integer
  const selectedAmPm = document.getElementById("ampm-dd").value;

  const hour = selectedHour;

  // Adjust selectedHour based on AM or PM
  if ((selectedAmPm === "pm" || selectedAmPm === "PM" )&& hour !== 12) {
    selectedHour += 12;
  } else if ((selectedAmPm === "am" || selectedAmPm === "AM")&& hour === 12) {
    // Here, if it's AM, you should not modify selectedHour
    selectedHour = 0;
  }



  console.log("After adjustment - selectedHour:", selectedHour);

  const selectedMinute = parseInt(document.getElementById("minute-dd").value, 10); // Convert to integer

 
  const daysButtons = document.querySelectorAll('.day-button');
  const selectedDays = [];

  daysButtons.forEach(button => {
    if (button.classList.contains('active')) {
      selectedDays.push(button.dataset.day);
    }
  });

  const amountToFeed = document.getElementById("amountToFeed").value;

  const schedulerSettings = {
    hour: selectedHour,
    minute: selectedMinute,
    ampm: selectedAmPm,
    days: selectedDays,
    amountToFeed: amountToFeed
  };
  console.log("Final schedulerSettings:", schedulerSettings);
  // Save scheduler settings to Firebase
  database.ref("schedulerSettings").set(schedulerSettings);
}

function toggleButton(button) {
  button.classList.toggle('active');
}

// Event listener for day buttons
const dayButtons = document.querySelectorAll('.day-button');
dayButtons.forEach(button => {
  button.addEventListener('click', function () {
    toggleButton(this);
  });
});






  // Event listener for the toggle button
  $(".toggle-btn").click(function () {
    toggleLedStatus();
  });

  // Event listener for the "MANUAL DISPENSE" button
  $("#manualDispenseBtn").click(function () {
    manualDispense();
  });


    // Event listener for the "MANUAL DISPENSE" button
    $("#PrimerButton").click(function () {
      Primer();
    });

  // Event listener for the "MANUAL DISPENSE" button
  $("#saveButton").click(function () {
   // manualDispense();
  });  




  // Function to toggle Led1Status in Firebase
  function toggleLedStatus() {
    var firebaseRef = firebase.database().ref().child("Led1Status");

    if (Led1Status === "1") {
      firebaseRef.set("0");
    } else {
      firebaseRef.set("1");
    }
  }

  // Function to toggle PrimerButton in Firebase
  function Primer() {
    var firebaseRef = firebase.database().ref().child("PrimerButton");

    if (PrimerButton === "1") {
      firebaseRef.set("0");
    } else {
      firebaseRef.set("1");
    }
  }





  // Function to update UI based on Led1Status
  function updateLedStatusUI() {
    if (Led1Status === "1") {
      document.getElementById("unact").style.display = "none";
      document.getElementById("act").style.display = "block";
    } else {
      document.getElementById("unact").style.display = "block";
      document.getElementById("act").style.display = "none";
    }
  }


  // Function to perform manual dispense
  function manualDispense() {
    var firebaseRef = firebase.database().ref().child("ManualStatus");

    // Toggle the manual dispense status
    if (ManualStatus === "1") {
      firebaseRef.set("0");
    } else {
      firebaseRef.set("1");
    }
  }















  // Event listener for the connection form submission
  document.getElementById("connectionForm").addEventListener("submit", function (event) {
    event.preventDefault();

    var ssid = document.getElementById("fname").value;
    var password = document.getElementById("lname").value;

    // Assuming you have a function to handle form submission and return an IP
    var ipAddress = submitFormAndReturnIP(ssid, password);

    // Display the returned IP address
    document.getElementById("ipDisplay").value = ipAddress;
  });



  // Event listener for the "TEST" button click in the scheduler
  document.getElementById("testButton").addEventListener("click", function () {
    // Get the value entered in the input for the amount to feed
    var amountToFeed = document.getElementById("amountToFeed").value;

    // Do something with the entered amount (you can customize this part)
    console.log("Amount to Feed: " + amountToFeed);
  });



  // Event listener for the "SAVE" button click in the scheduler
  document.getElementById("saveButton").addEventListener("click", function () {
    // Perform actions to save the scheduler settings (you can customize this part)
    console.log("Scheduler settings saved!");
  });


  // This function is just a placeholder. Replace it with your actual form submission logic.
  function submitFormAndReturnIP(ssid, password) {
    // Perform your form submission logic here, and return the obtained IP address.
    // For demonstration purposes, let's assume you have a function that returns a static IP.
    return "192.168.1.100";
  }
});



// Function to show the selected view
function showView(viewId) {
  // Hide all views
  const views = document.querySelectorAll('.visible, .hidden');
  views.forEach(view => view.classList.remove('visible'));
  views.forEach(view => view.classList.add('hidden'));

  // Show the selected view
  const selectedView = document.getElementById(viewId);
  selectedView.classList.remove('hidden');
  selectedView.classList.add('visible');
}




document.addEventListener('DOMContentLoaded', function () {
  const pressButton = document.getElementById('pressButton');

  pressButton.addEventListener('click', function () {
    const timestamp = new Date();
    const pressData = {
      timestamp: timestamp,
      buttonLabel: 'Press Me!',
    };

    // Add the data to Firestore collection named 'buttonPresses'
    db.collection('buttonPresses').add(pressData)
      .then((docRef) => {
        console.log('Button press recorded with ID:', docRef.id);
        // Fetch and display the data in the Feeding History Table
        fetchAndDisplayFeedingHistory();
      })
      .catch((error) => {
        console.error('Error adding button press:', error);
      });
  });

  // Fetch and display the data in the Feeding History Table on page load
  fetchAndDisplayFeedingHistory();
});

function fetchAndDisplayFeedingHistory() {
  const feedingHistoryTableBody = document.querySelector('#dashboardView tbody');

  // Clear existing table rows
  feedingHistoryTableBody.innerHTML = '';

  // Fetch data from Firestore collection 'buttonPresses'
  db.collection('buttonPresses')
  .orderBy('timestamp', 'desc') 
 
  .get()
  .then((querySnapshot) => {
    querySnapshot.forEach((doc) => {
      const data = doc.data();

      const amountToFeed = data.amountToFeed;
       
      const formattedTimestamp = new Date(data.timestamp.toDate()).toLocaleString();
        
      const tableRow = `<tr>
        <td>${formattedTimestamp}</td>
        <td>${amountToFeed}</td>
      </tr>`;
      feedingHistoryTableBody.innerHTML += tableRow;
    });

    feedingHistoryTableContainer.style.height = feedingHistoryTableBody.scrollHeight + 'px';

  })
  .catch((error) => {
    console.error('Error fetching feeding history:', error);
  });
}