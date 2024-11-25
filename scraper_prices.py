from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from fake_useragent import UserAgent
from selenium.webdriver.chrome.options import Options
import pandas as pd


def getPrices(date):

    chrome_options = Options()

    chrome_options.add_argument("--disable-geolocation")

    # Random user agent
    ua = UserAgent()
    user_agent = ua.random
    chrome_options.add_argument(f'user-agent={user_agent}')
    chrome_options.add_argument("--headless")  # Run in headless mode
    chrome_options.add_argument("--no-sandbox")
    chrome_options.add_argument("--disable-dev-shm-usage")

    driver = webdriver.Chrome(options=chrome_options)

    # Open the Nord Pool day-ahead prices page
    url = f'https://data.nordpoolgroup.com/auction/day-ahead/prices?deliveryDate={date}&currency=EUR&aggregation=Hourly&deliveryAreas=FI'
    driver.get(url)

    # Wait for the table to be loaded - waiting for the 'dx-data-grid' element to appear
    try:
        # Accept cookies if the prompt appears
        try:
            accept_cookies = WebDriverWait(driver, 10).until(
                EC.presence_of_element_located((By.XPATH, '//*[@id="mat-mdc-dialog-0"]/div/div/gdpr-dialog/div/div[2]/div/button[2]'))
            )
            accept_cookies.click()
        except Exception as e:
            print("Cookies accept button not found or already accepted.")

        # Waiting for the dx-data-grid element to be present
        WebDriverWait(driver, 20).until(
            EC.presence_of_element_located((By.XPATH, "//*[@id='grid-wrapper']/div[1]/dx-data-grid"))
        )

        # Now that the table has loaded, extract the rows
        rows = driver.find_elements(By.XPATH, "//*[@id='grid-wrapper']/div[1]/dx-data-grid//table/tbody/tr")

        data = []
        
        for row in rows:
            # Get each cell's text
            cells = row.find_elements(By.TAG_NAME, "td")
            row_data = [cell.text for cell in cells]
            data.append(row_data)
        
        
        df = pd.DataFrame(data)

        # Selecting only the values from row index 2 onwards in column 1
        price_values = df[1].iloc[2:26]
        price_values = price_values.dropna().reset_index(drop=True)
        price_values = price_values.str.replace(',', '.')
        price_values = pd.to_numeric(price_values, errors='coerce')

    finally:
        driver.quit()

    return price_values

#date = "today"
#print(getPrices(date))