<?php
// Magical scaling function.
function minute_to_value($minute, $units)  {
    if(!isset($minute->precipIntensity) || $minute->precipIntensity == 0) return 0;
    $t = $minute->precipIntensity;
    if($units != "us") $t /= 25.4;
    $t *= (1 - cos($minute->precipProbability * M_PI)) * 0.5;
    $t = 4*(1-exp(-2.209389806 * sqrt($t)));
    if($t <= 1) $t *= 0.15;
    elseif($t <= 2) $t = 0.15 + ($t-1)*(0.33-0.15);
    elseif($t <= 3) $t = 0.33 + ($t-2)*0.34;
    else $t = 0.67 + ($t-3)*(1-0.67);
    return $t;
}

define('API_KEY', '52c488b6e9f22a19b2ec31c083b62f46');
$payload = json_decode(file_get_contents('php://input'), true);
if(!$payload) die();
$payload[1] /= 10000;
$payload[2] /= 10000;
$url = "http://api.forecast.io/forecast/" . API_KEY . "/$payload[1],$payload[2]?units=$payload[3]&exclude=hourly,daily,alerts";
$forecast = json_decode(@file_get_contents($url));
if(!$forecast) {
    die();
}
$response = array();
$icons = array(
    'clear-day' => 0,
    'clear-night' => 1,
    'rain' => 2,
    'snow' => 3,
    'sleet' => 4,
    'wind' => 5,
    'fog' => 6,
    'cloudy' => 7,
    'partly-cloudy-day' => 8,
    'partly-cloudy-night' => 9
);
$icon_id = $icons[$forecast->currently->icon];
$temp = round($forecast->currently->temperature);
if($temp < 0) {
    $temp = -$temp;
    $temp = $temp | (1 << 10);
}
$temp = $temp | ($icon_id << 11);
$response[1] = array('S', $temp);

$has_precip = false;
if(isset($forecast->minutely) && $forecast->minutely->data) {
    $minutes = array();
    $cap = 0.4;
    foreach($forecast->minutely->data as $minute) {
        $value = minute_to_value($minute, $forecast->flags->units);
        $minutes[] = round($value * 255);
        if($value > 0) {
            $has_precip = true;
        }
    }
    if($has_precip) {
        $response[3] = array('d', base64_encode(call_user_func_array('pack', array_merge(array('C*'), $minutes))));
    }
}
// Don't cache if it's raining (because that'd screw up the graph)
if(!$has_precip)
    header("Cache-Control: max-age=1680");
print json_encode($response);
