function test() {
	print("this is test.");
}

var mei = new OpenJTalk();
mei.init("data/mei_normal", "openjtalk/open_jtalk_dic_utf_8-1.05");

var ir = new iRemocon();
ir.connect('192.168.0.7', 51013);

Julius.init("./data/setting.jconf");
Julius.onSpeechReady = function(){
	print("onSpeechReady");
};
Julius.onSpeechStart = function(){
	print("onSpeechStart");
};
Julius.onResult = function(result){
	print("onResult");
	if (/テレビ.*?つけ/.test(result)) {
		mei.talk(result + " を実行します");
		ir.send(1);
	}
};

Julius.start();
