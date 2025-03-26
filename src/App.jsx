import HomePage from "./home/HomePage";
import LoginPage from "./authentication/LoginPage";
import DefaultLayout from "./layouts/DefaultLayout";
import AuthLayout from "./layouts/auth/AuthLayout";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";

const App = () => {
  return (
    <Router>
      <Routes>
        <Route element={<DefaultLayout />}>
          <Route path="/" element={<HomePage />} />
        </Route>

        <Route element={<AuthLayout />}>
          <Route path="/login" element={<LoginPage />} />
        </Route>
      </Routes>
    </Router>
  );
};

export default App;
