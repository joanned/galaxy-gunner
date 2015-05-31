'use strict';

var express = require('express'),
	mongoose = require('mongoose'),
	mongooseAutoIncrement = require('mongoose-auto-increment'),
	env = require('node-env-file'),
	bodyParser = require('body-parser');

mongoose.connect('mongodb://localhost/galaxy-gunner');

var Schema = mongoose.Schema;

var highscoreSchema = new Schema({
	highscore: Number
});

var Highscore = mongoose.model('Highscore', highscoreSchema);

module.exports = Highscore;

var globalHighscore = new Highscore({
	highscore : 0
});

globalHighscore.save(function(err) {
	if (err) throw err;

	console.log('initial highscore saved');
})

//var port = process.env.PORT;
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
		console.log(highscore);
		highscore.highscore = newHighscore;
		highscore.save(function(err) {
			if (err) throw err;
			console.log('highscore saved:' + highscore);
		});
		if (err) throw err;
	});
}

function getHighscore (req, res) {
	console.log('get highscore start');
	Highscore.findOne({}, function(err, highscore) {
		console.log(highscore);
		if (err) throw err;
		console.log('highscore got:' + highscore.highscore);
	});
}

//env(__dirname + '/.env');

var server = app.listen(5321 , function() {
	
});
