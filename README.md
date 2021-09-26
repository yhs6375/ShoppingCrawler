C로 만든 웹 크롤러(멀티쓰레드 구현)입니다.</br>
크롤러의 동시 요청 처리를 위해 epoll 라이브러리를 사용했습니다.</br>
아래 사진처럼 json파일에 정보를 오픈마켓을 파싱하기 위한 정보를 집어넣고 해당사이트에서 HTML을 크롤링합니다.</br>
<img src="https://user-images.githubusercontent.com/53197506/87876769-4d67e580-ca15-11ea-9c21-8d44ba3afa86.PNG"/>
</br>
</br>
크롤러를 UDS로 node.js 서버와 연결 후 클라이언트(웹)에서 상품명을 입력 후 가져올 오픈마켓을 node.js서버로 요청해</br>
<img src="https://user-images.githubusercontent.com/53197506/87876950-4c838380-ca16-11ea-8e53-d7188d54ef48.PNG"/>
</br>
</br>
서버에서 uds를 이용해 크롤러로 아래처럼 데이터를 가져옵니다.
<img src="https://user-images.githubusercontent.com/53197506/87876772-4e991280-ca15-11ea-8387-bf86620aa1e4.PNG"/>
</br>
