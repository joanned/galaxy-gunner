'use strict';

var express = require('express'),
	mongoose = require('mongoose'),
	mongooseAutoIncrement = require('mongoose-auto-increment'),
	bodyParser = require('body-parser');

mongoose.connect('mongodb://localhost/galaxy-gunner');

var Schema = mongoose.Schema;

var highscoreSchema = new Schema({
	highscore: Number
});

var Highscore = mongoose.model('Highscore', highscoreSchema);

var globalHighscore = new Highscore({
	highscore : 0
});

globalHighscore.save(function(err) {
	if (err) {
		console.log(error);
	}

	console.log('initial highscore saved');
})

var app = express();
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
  extended: true
}));

app.post('/highscore', saveHighscore);
app.get('/highscore/:get', getHighscore);

function saveHighscore (req, res) {
	console.log('save highscore start');
	console.log(req.body);
	
	var newHighscore = parseInt(req.body.highscore);
	Highscore.findOne({}, function(err, highscore) {
		if (err) {
			res.sendStatus(500);
		}

		console.log(highscore);
		highscore.highscore = newHighscore;
		highscore.save(function(err) {
			if (err) {
				res.sendStatus(500);
			}

			res.sendStatus(200);
			console.log('highscore saved:' + highscore);
		});
		
		
	});
}

function getHighscore (req, res) {
	console.log('get highscore start');
	console.log(req.body);
	Highscore.findOne({}, function(err, highscore) {
		if (err) {
			res.sendStatus(500);
		}
		console.log('highscore got:' + highscore.highscore);
		res.json({
			highscore: highscore.highscore
		});
	});
}

//todo: make not variable
var server = app.listen(5454 , function() {
	console.log("server started listening");
});
