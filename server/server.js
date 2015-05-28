'use strict';

var express = require('express'),
	mongoose = require('mongoose'),
	mongooseAutoIncrement = require('mongoose-auto-increment'),
	env = require('node-env-file');

//var port = process.env.PORT;
var app = express();

//env(__dirname + '/.env');

app.get('/', function (req, res) {
  res.send(200);
  var highscore = req.body.highscore;
});

var server = app.listen(5321 , function() {
	
});
