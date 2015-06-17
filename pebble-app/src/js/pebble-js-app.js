'use strict';

Pebble.addEventListener('ready', onReady);
Pebble.addEventListener('appmessage', onAppMessageReceived);

var BASE_URL = 'http://0e6558ce.ngrok.io';

function onReady() {
  var data = {
    'jsReady': true
  };

  Pebble.sendAppMessage(data,
    function(e) {
      console.log('Send successful.');
    },
    function(e) {
      console.log('Send failed!');
    }
  );
}

function getHighscore() {
  var url = BASE_URL + '/highscore/:get';

  console.log('about to make xhr reuqest get highscore');

  xhrGetRequest(url, function (xhr) {
    if (xhr.readyState === 4) {
      var success = (xhr.status === 200);
      var json = JSON.parse(xhr.response);
      console.log('JS app got highscore from db:' + json.highscore);
      var data = {
        'getHighscore': json.highscore
      };

      Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);
    }
  });
}

function setHighscore(highscore) {

  var url = BASE_URL + '/highscore';
  var params = 'highscore=' + highscore.toString();

  console.log('about to make xhr reuqest send highscore');

  xhrPostRequest(url, params, function (xhr) {
    if (xhr.readyState === 4) {
      var success = (xhr.status === 200);
      console.log('send xhr success?' + success);
      console.log('status:' + xhr.status);
      var data = {
        'setHighscore': success
      };

      Pebble.sendAppMessage(data, onAppMessageSuccess, onAppMessageFailure);
    }
  });
}

function onAppMessageReceived(e) {
  console.log('e:' + e.payload);
  console.log('e.getHighscore:' + e.payload.getHighscore);
  console.log('e.setHighscore:' + e.payload.setHighscore);

  if (e.payload.getHighscore) {
    getHighscore();
  } else if (e.payload.setHighscore) {
    setHighscore(e.payload.setHighscore);
  }
}

function xhrPostRequest(url, params, postOnReadyStateChange) {
  var xhr = new XMLHttpRequest();

  xhr.onreadystatechange = function () {
    postOnReadyStateChange(xhr);
  };

  xhr.open('POST', url);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhr.setRequestHeader("Content-length", params.length);
  xhr.setRequestHeader("Connection", "close");
  xhr.send(params);

  return xhr;
}

function xhrGetRequest(url, getOnReadyStateChange) {
  var xhr = new XMLHttpRequest();

  xhr.onreadystatechange = function () {
    getOnReadyStateChange(xhr);
  };

  xhr.open('GET', url, true);
  xhr.send();
}

function onAppMessageSuccess() {
  console.log('onAppMessageSuccess');
}

function onAppMessageFailure() {
  console.log('onAppMessageFailure');
}