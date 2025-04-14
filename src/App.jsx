import HomePage from "./pages/home/HomePage";
import LoginPage from "./pages/authentication/LoginPage";
import RegisterPage from "./pages/authentication/RegisterPage";
import ProfilePage from "./pages/profile/ProfilePage";
import WatchPage from "./pages/watch/WatchPage";
import DefaultLayout from "./layouts/DefaultLayout";
import AuthLayout from "./layouts/auth/AuthLayout";
import StreamerProfilePage from "./pages/profile/StreamerProfilePage";
import { QueryClientProvider, QueryClient } from "@tanstack/react-query";
import { AuthProvider } from "./context/AuthContext";
import { BrowserRouter as Router, Route, Routes } from "react-router-dom";
import { FluentProvider, webLightTheme } from "@fluentui/react-components";

const queryClient = new QueryClient();

const App = () => {
  return (
    <FluentProvider theme={webLightTheme}>
      <QueryClientProvider client={queryClient}>
        <AuthProvider>
          <Router>
            <Routes>
              <Route element={<DefaultLayout />}>
                <Route path="/" element={<HomePage />} />
                <Route
                  path="/streamer-profile/:id"
                  element={<StreamerProfilePage />}
                />
                <Route path="/watch-test" element={<WatchPage />} />
              </Route>

              <Route element={<AuthLayout />}>
                <Route path="/login" element={<LoginPage />} />
                <Route path="/register" element={<RegisterPage />} />
                <Route path="/profile" element={<ProfilePage />} />
              </Route>
            </Routes>
          </Router>
        </AuthProvider>
      </QueryClientProvider>
    </FluentProvider>
  );
};

export default App;
