<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('sinaip')) {
	dl('sinaip.' . PHP_SHLIB_SUFFIX);
}
/*
$module = 'sinaip';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str\n";*/
$ips = sinaip_init("/var/www/stat/data/SinaIP.dat");
echo "Create OK? ".($ips === false ? "Failure" : "success")."\n";
echo "SinaIP Count: ".sinaip_count($ips)."\n";
$t1 = microtime(true);
for ($i=0;$i<10000;$i++)
{
	$_data = sinaip_search($ips,ip2long("183.61.1.27"));
	$ip = json_decode(gzuncompress($_data),true);
	/*
	fseek($this->fp,$this->index['ptr'.$key],SEEK_SET);
	$n['len'] = hexdec(bin2hex(fread($this->fp,4)));
	$_data = fread($this->fp,$n['len']);
	if (strlen($_data)!=$n['len'])
	{
		return false;
		//throw new Exception("Data file error.");
	}
	$ip = json_decode(gzuncompress($_data),true);*/
}
echo microtime(true)-$t1;
echo "\n";
sinaip_close($ips);
?>
