'use strict';

Pebble.addEventListener('ready', onReady);
Pebble.addEventListener('appmessage', onAppMessageReceived);

var BASE_URL = 'http://2bb6d195.ngrok.io';

function onReady() {
  var data = {
    jsReady: true
  };

  Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);

}

function getHighscore() {
  var url = BASE_URL + '/highscore';

  xhrPostRequest(url, function (xhr) {
    if (xhr.readyState === 4) {
      var success = (xhr.status === 200);
      console.log('got highscore from db:' + xhr.responseText);
      var data = {
        updateSuccessful: success
      };

      Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);
    }
  });
}

function setHighscore(highscore) {
  var xhr = new XMLHttpRequest();

  var url = BASE_URL + '/highscore';
  var params = 'highscore=' + highscore.toString();

  console.log('about to make xhr reuqest send highscore');

  xhrPostRequest(url, params, function (xhr) {
    if (xhr.readyState === 4) {
      var success = (xhr.status === 200);
      console.log('send xhr success?' + success);
      var data = {
        updateSuccessful: success
      };

      Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);
    }
  });
}

function onAppMessageReceived(e) {
  console.log('e:' + e.payload);
  console.log('e.getHighscore:' + e.payload.getHighscore);
  console.log('e.setHighscore:' + e.payload.setHighscore);
  setHighscore(5);
  //getHighscore();

  //todo: tingssss
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

// function xhrGetRequest(url, onReadyStateChange) {
//   var xhr = new XMLHttpRequest();

//   xhr.onreadystatechange = function () {
//     onReadyStateChange(xhr);
//   };

//   xhr.open('GET', url);
//   xhr.send(null);
// }



function onAppMessageSuccess() {
  console.log('onAppMessageSuccess');
}

function onAppMessageFailure() {
  console.log('onAppMessageFailure');
}