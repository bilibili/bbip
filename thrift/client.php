<?php  
$GLOBALS['THRIFT_ROOT'] = '/usr/local/src/thrift-0.9.3/lib/php/lib/Thrift';  

		$ts = microtime(true);
require_once $GLOBALS['THRIFT_ROOT'].'/ClassLoader/ThriftClassLoader.php';
use Thrift\ClassLoader\ThriftClassLoader;

$GEN_DIR = realpath(dirname(__FILE__)).'/gen-php';
$loader = new ThriftClassLoader();
$loader->registerNamespace('Thrift', $GLOBALS['THRIFT_ROOT']."/../");
$loader->registerDefinition('com', $GEN_DIR);
$loader->register();

use Thrift\Protocol\TBinaryProtocolAccelerated;
use Thrift\Protocol\TBinaryProtocol;
use Thrift\Transport\TSocket;
use Thrift\Transport\TPhpStream;
use Thrift\Transport\TFramedTransport;
use Thrift\Transport\TBufferedTransport;
use Thrift\TMultiplexedProcessor;

use com\bilibili\bbipClient;

// for ($l=0;$l<20;$l++)
{
	// if (pcntl_fork()==0)
	{

		$thrif_server_url = '172.16.1.254';
		$transport = new TFramedTransport(new TSocket($thrif_server_url, 12233));
		echo "\nrun at :".(microtime(true)-$ts)."s\n";
		$transport->open();  
		 
		// if (class_exists("TBinaryProtocolAccelerated"))
		{
			$protocol = new TBinaryProtocolAccelerated($transport);  			
		}
		// else
		{
			// $protocol = new TBinaryProtocol($transport);  
		}
		  
		$client= new bbipClient($protocol, $protocol);

		$fb_client = new FacebookServiceClient($protocol, $protocol);
		// $t1 = microtime(true);
		// for ($i=0;$i<10000;$i++)
		{
			// echo "Doing {$i}\r";
			// $client->count();
			echo ($client->GetZoneID("223.5.5.5")."\n");
			echo($client->GetZoneID("14.136.131.1")."\n");
			print_r($client->query("183.129.129.129"));	
			echo ($client->query_string("183.129.129.129"));	
		}
		echo $fb_client->getName()."\n";
		// echo ($i/(microtime(true)-$t1))."pps\n";
		$transport->close();
		echo "\nrun at :".(microtime(true)-$ts)."s\n";
		exit;
	}
}
?>
