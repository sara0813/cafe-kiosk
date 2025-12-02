# ☕ Cafe Kiosk Simulator

## 📌 Overview
Cafe Kiosk Simulator는 실제 카페 키오스크의 주문 흐름을 콘솔 환경에서 구현한 프로그램입니다.  
사용자(User) 모드와 관리자(Admin) 모드를 분리하여 **주문·결제, 재고·메뉴 관리, 환불 처리, 매출 통계 조회** 등을 하나의 시스템에서 관리할 수 있습니다.<br>

모든 데이터는 파일 기반으로 관리되며, 기능별로 코드를 모듈화해 유지보수와 확장성을 고려하여 설계되었습니다.

---

## 📁 Project Structure

```text
project/
│
├── data/                     # 프로그램 실행에 필요한 데이터 파일
│    ├── menus/               # 메뉴 및 가격 정보
│    ├── stocks/              # 메뉴별 재고
│    └── logs/                # 주문 및 환불 로그
│         ├── Orders.log
│         └── refunds.log
│
├── src/                      # 주요 기능 소스 코드
│    ├── main.c
│    ├── ... (기타 모듈)
│
├── include/                  # 각 모듈용 헤더 파일
│
└── Makefile
```
---

## ✨ Features
- # **User Mode**<br>
카테고리/메뉴 리스트 확인<br>
옵션 및 수량 선택<br>
장바구니 기반 주문 처리<br>
결제 및 주문 완료 <br>
주문 내역을 Orders.log에 자동 기록

- # **Admin Mode**<br>
  ID: team2<br>
  PW: 1234<br>

1. 매출 현황 (Sales / Statistics) <br>
2. 메뉴 / 가격 수정 (Menu / Price Edit) <br>
3. 재고 관리 (Stock Management) <br>
4. 환불 관리 (Order / Refund Management) <br>

- # **Input Timeout Behavior** <br>
주문 과정에서 20초 동안 입력이 없으면 경고 메시지를 표시합니다. <br>
25초 동안 입력이 없으면면 현재 주문을 자동 취소하고 메인 화면으로 돌아갑니다. <br>
오랫동안 방치되더라도 프로그램이 멈추지 않도록 설계되었습니다. <br>

---

## 🛠 Build & Run
```bash
make
```

```bash
./cafe-kiosk
```
