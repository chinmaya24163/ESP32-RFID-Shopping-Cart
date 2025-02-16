#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 22  // RST pin for ESP32 WROOM-32
#define SS_PIN 5    // SS (SDA) pin

MFRC522 mfrc522(SS_PIN, RST_PIN);

struct Product {
    const char* name;
    float price;
};

// Three RFID product cards
const struct {
    const byte uid[4];
    Product product;
} productMap[] = {
    {{0x7B, 0xAE, 0xE9, 0x0A}, {"Surf Excel", 130.0}},
    {{0x46, 0x03, 0x3F, 0xF8}, {"Cornflakes", 50.0}},
    {{0x96, 0xD2, 0x38, 0xF8}, {"Milk", 40.0}}
};

// Fourth RFID card acts as payment verifier
const byte paymentCardUID[4] = {0x79, 0x0E, 0x8D, 0xBB};  // Change this to match your payment card

// Variables to track scanned items
const int MAX_ITEMS = 3;
Product scannedProducts[MAX_ITEMS] = {{"", 0.0}, {"", 0.0}, {"", 0.0}};
float totalPrice = 0.0;

void setup() {
    Serial.begin(115200);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("Ready to scan RFID tags...");
}

void loop() {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        delay(100);
        return;
    }

    // Check if scanned card is the payment verifier
    if (memcmp(mfrc522.uid.uidByte, paymentCardUID, 4) == 0) {
        processPayment();
        delay(1000);
        return;
    }

    Product scannedProduct = {"Unknown", 0.0};
    int index = -1;

    // Check if scanned card matches a product
    for (int i = 0; i < sizeof(productMap) / sizeof(productMap[0]); i++) {
        if (memcmp(mfrc522.uid.uidByte, productMap[i].uid, 4) == 0) {
            scannedProduct = productMap[i].product;
            index = i;
            break;
        }
    }

    if (scannedProduct.price > 0.0) {
        // Check if the item is already in the cart
        if (scannedProducts[index].price > 0.0) {
            totalPrice -= scannedProducts[index].price;
            Serial.print("Removed: ");
            Serial.println(scannedProducts[index].name);

            scannedProducts[index] = {"", 0.0};  // Remove item
        } else {
            totalPrice += scannedProduct.price;
            scannedProducts[index] = scannedProduct;

            Serial.print("Added: ");
            Serial.println(scannedProduct.name);
        }

        Serial.print("Total Cost: ₹");
        Serial.println(totalPrice, 2);
    } else {
        Serial.println("Unknown card detected!");
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(1000);
}

// Function to process payment when payment card is scanned
void processPayment() {
    if (totalPrice == 0) {
        Serial.println("No items in the cart to process payment.");
        return;
    }

    Serial.println("Payment Card Detected...");
    Serial.print("Total Paid: ₹");
    Serial.println(totalPrice, 2);
    Serial.println("Payment Successful!");

    // Reset cart after payment
    for (int i = 0; i < MAX_ITEMS; i++) {
        scannedProducts[i] = {"", 0.0};
    }
    totalPrice = 0.0;

    Serial.println("Cart Cleared! Ready for new scan.");
}
