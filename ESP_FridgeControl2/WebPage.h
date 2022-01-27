//***********************************************************************************************
//***********************************************************************************************
const char index_html[] PROGMEM = R"rawliteral(
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet"
href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr"
crossorigin="anonymous">
<style>
html {
font-family: Arial;
display: inline-block;
margin: 0px auto;
text-align: center;
}
h2 { font-size: 3.0rem; }
p { font-size: 3.0rem; }
.units { font-size: 1.2rem; }
.dht-labels{
font-size: 1.5rem;

vertical-align:middle;
padding-bottom: 15px;
}
</style>
<title>Main</title>
</head>
<body>
<h2>Kegerator Temperature Controller</h2>
<p> <span class="dht-labels">Frdige Temperature</span> <span id="temperature">%TEMPERATURE%</span> - Fahrenheit </p>
<p> <span class="dht-labels">Fridge Humidity</span> <span id="humidity">%HUMIDITY%</span> - Percent</p>
<p> <span class="dht-labels">Spare Temperature</span> <span id="sparetemp">%SpareTemp%</span> - Fahrenheit </p>
<table
style="text-align: left; margin-left: auto; margin-right: auto; width: 901px;"
border="1" cellpadding="2" cellspacing="1">
<caption>
<br>
</caption> <tbody>
<tr>
<td style="vertical-align: top; width: 140px;"><br></td>
<td style="vertical-align: top; width: 221px;">Fridge Temp<br></td>
<td style="vertical-align: top; width: 284px;">Fridge Humidity<br></td>
<td style="vertical-align: top; width: 186px;">Spare Temp<br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Current Value<br>
</td>
<td style="vertical-align: top; width: 221px;"><span id="temperature2">%TEMPERATURE%</span> </td>
<td style="vertical-align: top; width: 284px;"><span id="humidity2">%HUMIDITY%</span></td>
<td style="vertical-align: top; width: 186px;"><span id="sparetemp2">%SpareTemp%</span></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Target Max<br></td>
<td style="vertical-align: top; width: 221px;">%TempTarget%<br></td>
<td style="vertical-align: top; width: 284px;">%HumidTarget%<br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Target Range<br></td>
<td style="vertical-align: top; width: 221px;">%TempLowOffset%<br></td>
<td style="vertical-align: top; width: 284px;">%HumidLowOffset%<br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Min On<br></td>
<td style="vertical-align: top; width: 221px;">%TempMinRunSeconds%<br></td>
<td style="vertical-align: top; width: 284px;">%HumidMinRunSeconds%<br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Status<br></td>
<td style="vertical-align: top; width: 221px;"><span id="tempstatus2">%TempStatus%</span><br></td>
<td style="vertical-align: top; width: 284px;"><span id="humidstatus2">%HumidStatus%</span><br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">ProtectSeconds<br></td>
<td style="vertical-align: top; width: 221px;"><span id="TempProtectDelaySeconds">%TempProtectDelaySeconds%</span><br></td>
<td style="vertical-align: top; width: 284px;"><span id="HumidProtectDelaySeconds">%HumidProtectDelaySeconds%</span><br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Last Run Duration<br></td>
<td style="vertical-align: top; width: 221px;"><span id="TempLastOnTime">%TempLastOnTime%</span><br></td>
<td style="vertical-align: top; width: 284px;"><span id="HumidLastOnTime">%HumidLastOnTime%</span><br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top; width: 140px;">Time Since Run<br></td>
<td style="vertical-align: top; width: 221px;"><span id="TempOffTime">%TempOffTime%</span><br></td>
<td style="vertical-align: top; width: 284px;"><span id="HumidOffTime">%HumidOffTime%</span><br></td>
<td style="vertical-align: top; width: 186px;"><br></td>
</tr>
<tr>
<td style="vertical-align: top;">Cycle Count<br></td>
<td style="vertical-align: top;"><span id="TempCycleCount">%TempCycleCount%</span><br></td>
<td style="vertical-align: top;"><span id="HumidCycleCount">%HumidCycleCount%</span><br></td>
<td style="vertical-align: top;"><br></td>
</tr>
</tbody>
</table>
<br>
<br>
<form action="/get"> InputSetTemp: <input name="InputSetTemp"
type="text"> <input value="Submit" type="submit"> </form>
<script>


//test this stuff to see if I can do multiples and split
//Expecting Temperature|Humidity|sparetemp|TempStatus|Humidstatus|TemplastonTime|HumidlastOnTime|TempOffTime|HumidOffTime|TempcycleCount|HumidCycleCount

setInterval(function ( ) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
	if (this.readyState == 4 && this.status == 200) {
		var ajaxResult = this.responseText;
		var tmpArray = ajaxResult.split("|");
		
		var SecTempLastOn = tmpArray[5]/60.0;
    var SecTempOffTime = tmpArray[7]/60.0;
		document.getElementById("temperature").innerHTML = tmpArray[0];
		document.getElementById("temperature2").innerHTML = tmpArray[0];
		document.getElementById("humidity").innerHTML = tmpArray[1];
		document.getElementById("humidity2").innerHTML = tmpArray[1];
		document.getElementById("sparetemp").innerHTML = tmpArray[2];
		document.getElementById("sparetemp2").innerHTML = tmpArray[2];
		document.getElementById("tempstatus2").innerHTML = tmpArray[3];
		document.getElementById("humidstatus2").innerHTML = tmpArray[4];
		//document.getElementById("TempLastOnTime").innerHTML = tmpArray[5];
		document.getElementById("TempLastOnTime").innerHTML = SecTempLastOn
		document.getElementById("HumidLastOnTime").innerHTML = tmpArray[6];
    document.getElementById("TempOffTime").innerHTML = SecTempOffTime;
		//document.getElementById("TempOffTime").innerHTML = tmpArray[7];
		document.getElementById("HumidOffTime").innerHTML = tmpArray[8];
		document.getElementById("TempCycleCount").innerHTML = tmpArray[9];
		document.getElementById("HumidCycleCount").innerHTML = tmpArray[10];
	}
};
xhttp.open("GET", "/UpdateGroup", true);
xhttp.send();
}, 30000 ) ;

</script>
</body>
</html>
)rawliteral";
//***********************************************************************************************
//***********************************************************************************************




// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("temperature").innerHTML = this.responseText;
// document.getElementById("temperature2").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/temperature", true);
// xhttp.send();
// }, 100000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("humidity").innerHTML = this.responseText;
// document.getElementById("humidity2").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/humidity", true);
// xhttp.send();
// }, 100000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("sparetemp").innerHTML = this.responseText;
// document.getElementById("sparetemp2").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/sparetemp", true);
// xhttp.send();
// }, 100000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("tempstatus2").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/tempstatus2", true);
// xhttp.send();
// }, 500000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("humidstatus2").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/humidstatus2", true);
// xhttp.send();
// }, 500000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("TempLastOnTime").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/TempLastOnTime", true);
// xhttp.send();
// }, 1000000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("HumidLastOnTime").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/HumidLastOnTime", true);
// xhttp.send();
// }, 1000000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("TempOffTime").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/TempOffTime", true);
// xhttp.send();
// }, 500000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("HumidOffTime").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/HumidOffTime", true);
// xhttp.send();
// }, 500000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("TempCycleCount").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/TempCycleCount", true);
// xhttp.send();
// }, 1000000 ) ;

// setInterval(function ( ) {
// var xhttp = new XMLHttpRequest();
// xhttp.onreadystatechange = function() {
// if (this.readyState == 4 && this.status == 200) {
// document.getElementById("HumidCycleCount").innerHTML = this.responseText;
// }
// };
// xhttp.open("GET", "/HumidCycleCount", true);
// xhttp.send();
// }, 10000 ) ;
