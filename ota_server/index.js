const express = require('express');
var path = require('path'); 

var app = express();

// start the server
app.use(express.json());

var server = app.listen(8081, function () {
    var host = server.address().address
    var port = server.address().port

    console.log(`ota_server listening at port ${port}`)
});

/**
 * GET /binary
 * 
 * Serves the binary file for OTA, this is just a test target for now 
 */
app.get('/binary.bin', function (req, res) {
    var options = { 
        root: path.join(__dirname) 
    }; 

    var fileName = 'binary.bin'; 
    res.sendFile(fileName, options, function (err) { 
        if (err) { 
            console.log(`Error: ${err}`); 
        } else { 
            console.log('Sent:', fileName); 
        } 
    });
})