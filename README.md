# AlirezaHttpClient

header-only asynchronous/synchronous http client for c++11 supporting Windows.

Simple example:

    try {
		AlirezaHttpClient a;
		a.setMethod(AlirezaHttpClient::GET);
		a.setParam("license", "12345");
		a.setPath("/Service_Portal.asmx/ReceiveANPR");
		a.sendAsync("www.sku.ac.ir", [] (string header,string body) { cout << header << endl << body });
		a.waitForResult();
	}
	catch (runtime_error e) {
		cout << e.what() << endl;
	}

Multi-part http request and POST request are in WIP (work in progress).
