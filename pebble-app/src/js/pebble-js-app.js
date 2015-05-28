'use strict';

Pebble.addEventListener('ready', onReady);
Pebble.addEventListener('appmessage', onAppMessageReceived);

var BASE_URL = 'http://2bb6d195.ngrok.io';

function onReady() {
	 console.log('onready');
  var data = {
    jsReady: true
  };

  Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);

}

function onAppMessageReceived(e) {
	if (e.payload.getHighscore) {

	} else if (e.payload.setHighscore) {

	}

	xhrPostRequest(BASE_URL, params, function (xhr) {
	    if (xhr.readyState === 4) {
	      var success = (xhr.status === 200);

	      var data = {
	        updateSuccessful: success
	      };

	      Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);
	    }
  	});
	
}

function xhrPostRequest(url, params, onReadyStateChange) {
  var xhr = new XMLHttpRequest();

  xhr.onreadystatechange = function () {
    onReadyStateChange(xhr);
  };

  xhr.open('POST', url);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhr.setRequestHeader("Content-length", params.length);
  xhr.setRequestHeader("Connection", "close");
  xhr.send(params);

  return xhr;
}



function onAppMessageSuccess() {
  console.log('onAppMessageSuccess');
}

function onAppMessageFailure() {
  console.log('onAppMessageFailure');
}